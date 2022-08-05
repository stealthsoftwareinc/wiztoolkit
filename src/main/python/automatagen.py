#! /usr/bin/python3

# Copyright (C) 2020 Stealth Software Technologies, Inc.

# This is a python helper to print out C++ code which evaluates some DFAs.
# A state-machine graph is built in python memory, and then serialized into
# a C++ function which has a loop and a bunch of if/cases statements to
# match text against the DFA.
#
# Each DFA generated is represented by an (internal) enum of states, along
# with a (public) function function with the encoding of state transitions.
#
# NOTE: that certain lines of this program use bitwise operations as logicals
# for the reason that they avoid short-circuit evaluation. In this case,
# because each comparison is so small and has no side-effects, short-circuit
# is slightly slower, I'm guessing because it hurts branch-prediction.
# In cases where bitwise operations are used, the code generator correctly
# inserts additional parenthesis to avoid operator precedence issues.

AllAutomatas = list()
AutomataName = str()

#* The Automata class represents a state in a DFA, as either an accepting
#* or non-accepting state, and a list of allowable transitions. Each
#* transition represents a condition as well as a pointer to the next state.
#* Each Automata object has an ID unique to its state-graph. Along with a 

class Automata:
  def __init__(self):
    self.accept = False
    self.acceptVal = "false"
    self.transitions = list()
    self.num = len(AllAutomatas)
    AllAutomatas.append(self)
  def findCharTrans(self, c):
    for t in self.transitions:
      if t.ttype == "char" and t.character == c:
        return t
    return None
  #* Inserts a sequence of states which would match an exact string.
  def insertString(self, string, acceptVal = "true"):
    if len(string) == 0:
      self.setAccept(acceptVal)
      return
    trans = self.findCharTrans(string[0])
    if trans is None:
      trans = CharTransition(Automata(), False, string[0])
      self.transitions.append(trans)
    trans.nextState.insertString(string[1:len(string)], acceptVal)
  def toString(self):
    return AutomataName + str(self.num)
  #* Returns a list of transitions who's next state is this state.
  def selfReferences(self):
    ret = list()
    for i in self.transitions:
      if i.nextState is self:
        ret.append(i)
    return ret
  #* Returns a list of transitions who's next state is not this state.
  def nonSelfReferences(self):
    ret = list()
    for i in self.transitions:
      if i.nextState is not self:
        ret.append(i)
    return ret
  #* Indicates if this automata has a transition which references itself.
  def hasSelfReference(self):
    for i in self.transitions:
      if i.nextState is self:
        return True
    return False
  #* Sets this state to accept.
  def setAccept(self, acceptVal = "true"):
    self.accept = True
    self.acceptVal = acceptVal

#* The transition is a condition which guards a next state.
#* This is an abstract class which encapsulates just the state
#* and a negation of the condition.
#*
#* The abstract condition() method must return a C++ expression to match
#* the condition for the next state.
class Transition:
  def __init__(self, ns, neg, t):
    self.nextState = ns
    self.negate = neg
    self.ttype = t
  #* Prints a C++ if-statement which handles the condition and changes
  #* the state to the next state.
  def toString(self):
    ret = str()
    ret = ret + "      if(" + self.condition() +  ") {\n"
    ret = ret + "        state = " + self.nextState.toString() + ";\n"
    ret = ret + "        ctx->place += " + str(self.numAdvance()) + ";\n"
    ret = ret + "        break;\n      }\n"
    return ret
  def numAdvance(self):
    return 1

#* A char transition has a condition which matches an exact character.
class CharTransition(Transition):
  def __init__(self, ns, neg, c):
    Transition.__init__(self, ns, neg, "char")
    self.character = c
  def condition(self, offset = 0):
    ret = str() + "ctx->buffer[ctx->place + " + str(offset) + "] "
    if self.negate:
      ret = ret + "!= "
    else:
      ret = ret + "== "
    ret = ret + "\'" + self.character + "\'"
    return ret

#* The range transition's condition matches a range of characters,
#* for example 0 through 9.
class RangeTransition(Transition):
  def __init__(self, ns, neg, s, e):
    Transition.__init__(self, ns, neg, "range")
    self.start = s
    self.end = e
  def condition(self, offset = 0):
    ret = str() + "(ctx->buffer[ctx->place + " + str(offset) + "] "
    if self.negate:
      ret = ret + "< "
    else:
      ret = ret + ">= "
    ret = ret + "\'" + self.start + "\') "
    if self.negate:
      ret = ret + "| "
    else:
      ret = ret + "& "
    ret = ret + "(ctx->buffer[ctx->place + " + str(offset) + "] "
    if self.negate:
      ret = ret + "> "
    else:
      ret = ret + "<= "
    ret = ret + "\'" + self.end + "\')"
    return ret

#* The lookahead transition matches a sequence of states each with a single,
#* acyclic, and non-accepting transition. Typically transitions are entered
#* without lookahead, and lookaheads are determined just before the DFA is
#* serialized.
#*
#* Lookaheads are used to reduce the loop iterations used by a DFA.
class LookaheadTransition(Transition):
  def __init__(self, ns):
    Transition.__init__(self, ns, False, "lookahead")
    self.lookaheads = list()
  def condition(self):
    ret = "(" + self.lookaheads[0].condition() + ")"
    if len(self.lookaheads) > 2:
      # first and second letters can short circuit, because if the first
      # letter is wrong then probably it is a different transition.
      ret = ret + "\n          && ((" + self.lookaheads[1].condition(1) + ")"
      for offset in range(2, len(self.lookaheads)):
        # remaining characters should not short circuit, because if the first
        # letter matched, then a subsequent non-match would be a parse
        # failure, which doesn't need to be as fast.
        ret = ret + "\n            & ("
        ret = ret + self.lookaheads[offset].condition(offset) + ")"
      ret = ret + ")"
    else:
        ret = ret + " && (" + self.lookaheads[1].condition(1) + ")"
    return ret
  def numAdvance(self):
    return len(self.lookaheads)

def numIncomingTransitions(state):
  incomingTransitions = 0
  for automata in AllAutomatas:
    for transition in automata.transitions:
      if transition.nextState is state:
        incomingTransitions = incomingTransitions + 1
  return incomingTransitions

#* This function converts sequences of single state transitions into
#* lookahead transitions, which are bunched into a single condition.
#* It also removes states whose conditions are always checked by
#* prior lookahead conditions.
def lookahead():
  i = 0
  while i < len(AllAutomatas):
    automata = AllAutomatas[i]
    for j in range(0, len(automata.transitions)):
      transition = automata.transitions[j]
      nextState = transition.nextState
      lookaheadTrans = LookaheadTransition(nextState)
      lookaheadTrans.lookaheads.append(transition)
      # TODO: this needs a proper cycle checking currently 
      while 1 >= numIncomingTransitions(nextState) \
          and 1 == len(nextState.transitions) \
          and nextState.transitions[0].nextState is not nextState \
          and nextState.transitions[0].nextState is not automata \
          and not nextState.accept \
          and not automata.accept:
        nextTrans = nextState.transitions[0]
        if nextTrans.ttype == "lookahead":
          lookaheadTrans.lookaheads.extend(nextTrans.lookaheads)
        else:
          lookaheadTrans.lookaheads.append(nextTrans)
        if nextState in AllAutomatas:
          AllAutomatas.remove(nextState)
        nextState = nextTrans.nextState
      lookaheadTrans.nextState = nextState
      if len(lookaheadTrans.lookaheads) > 1:
        automata.transitions[j] = lookaheadTrans
    i = i + 1

#* Prints out a list of the states, called by printAutomata.
def printStates():
  ret = str()
  ret = ret + "enum " + AutomataName + "States {\n"
  for a in AllAutomatas:
    ret = ret + "  " + a.toString() +",\n"
  ret = ret + "};\n"
  return ret

#* Prints out an automata function. if "body" is false, then just a
#* function header is printed, otherwise the whole function is given.
#* "rtype" is the return type of the function, and "fail" is the value
#* returned when a parse failure is encountered.
def printAutomata(body, rtype = "bool", fail = "false"):
  ret = str()
  if body:
    lookahead()
    ret = printStates()
  ret = ret + "\nALWAYS_INLINE static inline "
  ret = ret + rtype + " " + AutomataName + "(AutomataCtx* ctx"
  ret = ret + ") noexcept"
  if not body:
    ret = ret + ";\n"
    return ret
  ret = ret + " {\n"
  ret = ret + "  " + AutomataName + "States state = \n"
  ret = ret + "    " + AllAutomatas[0].toString() + ";\n"
  ret = ret + "  while (true) {\n"
  ret = ret + "    switch(state) {\n"
  # This loop prints each state.
  for a in AllAutomatas:
    ret = ret + "    case " + a.toString() + ": {\n"
    # If the loop has transitions to itself, it can make small loop to
    # avoid the bigger loop with more checks.
    # TODO: can lookaheads be self-refs??
    selfRefs = a.selfReferences()
    if len(selfRefs) > 0:
      first = True
      for sr in selfRefs:
        if first and len(selfRefs) == 1:
          ret = ret + "      while(" + sr.condition()
        elif first:
          ret = ret + "      while((" + sr.condition() + ")"
        else:
          ret = ret + "\n          || (" + sr.condition() + ")"
        first = False
      ret = ret + ") {\n"
      ret = ret + "        ++ctx->place;\n"
      ret = ret + "      };\n"
    # All other transitions (including lookaheads) just change the state
    # and iterate the big loop.
    for t in a.nonSelfReferences():
      ret = ret + t.toString()
    # If all possible transitions are exhausted, then either accept or
    # reject the input.
    if a.accept:
      ret = ret + "      return " + a.acceptVal + ";\n"
    else:
      ret = ret + "      log_error(\"DFA state " + a.toString()
      ret = ret + " rejected \'%c\' \'%.16s...\'\\n\", "
      ret = ret + "ctx->buffer[ctx->place], &ctx->buffer[ctx->place + 1]);\n"
      ret = ret + "      return " + fail + ";\n"
    ret = ret + "    }\n"
  ret = ret + "    }\n"
  ret = ret + "  }\n"
  # I'm pretty sure this is unreachable, but C++ requests a return here.
  ret = ret + "  return " + fail + ";\n"
  ret = ret + "}\n\n"
  return ret

# ==== Write includes and headers ====

th = open("target/generated/wtk/irregular/Automatas.t.h", "w")
h = open("target/generated/wtk/irregular/Automatas.h", "w");

th.write("namespace wtk {\n")
th.write("namespace irregular {\n\n")

h.write("#ifndef WIZTOOLKIT_AUTOMATA_H_\n#define WIZTOOLKIT_AUTOMATA_H_\n\n")
h.write("#include <string>\n\n")
h.write("#include <wtk/IRParameters.h>\n")
h.write("#include <wtk/utils/hints.h>\n\n")
h.write("#include <wtk/irregular/AutomataCtx.h>\n")
h.write("#include <wtk/irregular/ParseEnums.h>\n\n")

h.write("namespace wtk {\n")
h.write("namespace irregular {\n\n")

# =======================
# ==== Common Tokens ====
# =======================

# ==== Whitespace ====

AutomataName = "whitespace"
AllAutomatas = list()

whitespace = Automata()
whitespace.setAccept()
whitespace.transitions.append(CharTransition(whitespace, False, " "));
whitespace.transitions.append(CharTransition(whitespace, False, "\\n"));
whitespace.transitions.append(CharTransition(whitespace, False, "\\r"));
whitespace.transitions.append(CharTransition(whitespace, False, "\\t"));

cmmt1 = Automata()
cmmt2 = Automata()

whitespace.transitions.append(CharTransition(cmmt1, False, "/"))
cmmt1.transitions.append(CharTransition(cmmt2, False, "/"))

cmmt2.transitions.append(CharTransition(cmmt2, True, "\\n"))
cmmt2.transitions.append(CharTransition(whitespace, False, "\\n"))

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== Numeric Literals ====

def numericAutomata(automata, returns):
  numLit = Automata()
  numLit0 = Automata()
  numLit0.setAccept(returns["dec"])

  automata.transitions.append(CharTransition(numLit0, False, "0"))

  numLitX = Automata()
  numLitXn = Automata()
  numLitXn.setAccept(returns["hex"])

  numLit0.transitions.append(CharTransition(numLitX, False, "x"))
  numLit0.transitions.append(CharTransition(numLitX, False, "X"))

  numLitX.transitions.append(RangeTransition(numLitXn, False, "0", "9"))
  numLitX.transitions.append(RangeTransition(numLitXn, False, "A", "F"))
  numLitX.transitions.append(RangeTransition(numLitXn, False, "a", "f"))

  numLitXn.transitions.append(RangeTransition(numLitXn, False, "0", "9"))
  numLitXn.transitions.append(RangeTransition(numLitXn, False, "A", "F"))
  numLitXn.transitions.append(RangeTransition(numLitXn, False, "a", "f"))

  numLitO = Automata()
  numLitOn = Automata()
  numLitOn.setAccept(returns["oct"])

  numLit0.transitions.append(CharTransition(numLitO, False, "o"))

  numLitO.transitions.append(RangeTransition(numLitOn, False, "0", "7"))
  numLitOn.transitions.append(RangeTransition(numLitOn, False, "0", "7"))

  numLitB = Automata()
  numLitBn = Automata()
  numLitBn.setAccept(returns["bin"])

  numLit0.transitions.append(CharTransition(numLitB, False, "b"))
  numLit0.transitions.append(CharTransition(numLitB, False, "B"))

  numLitB.transitions.append(RangeTransition(numLitBn, False, "0", "1"))
  numLitBn.transitions.append(RangeTransition(numLitBn, False, "0", "1"))

  numLitDec = Automata()
  numLitDec.setAccept(returns["dec"])

  automata.transitions.append(RangeTransition(numLitDec, False, '1', '9'))
  numLitDec.transitions.append(RangeTransition(numLitDec, False, '0', '9'))

AutomataName = "numericLiteral"
AllAutomatas = list()

numReturns = dict()
numReturns["dec"] = "NumericBase::dec"
numReturns["hex"] = "NumericBase::hex"
numReturns["bin"] = "NumericBase::bin"
numReturns["oct"] = "NumericBase::oct"

numLit = Automata()
numericAutomata(numLit, numReturns)

th.write(printAutomata(True, "NumericBase", "NumericBase::invalid"))
h.write(printAutomata(False, "NumericBase", "NumericBase::invalid"))

# =============================
# ==== Header/front matter ====
# =============================

# ==== Version ====

AutomataName = "versionKw"
AllAutomatas = list()

version = Automata()
version.insertString("version");

th.write(printAutomata(True))
h.write(printAutomata(False))


# ==== Field (Keyword) ====

AutomataName = "fieldKw"
AllAutomatas = list()

field = Automata()
field.insertString("field")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== Decimal Literal (for version) ====

AutomataName = "decLiteral"
AllAutomatas = list()

decLiteral = Automata()
decLiteral0 = Automata()
decLiteral0.setAccept()
decLiteral19 = Automata()
decLiteral19.setAccept()
decLiteral.transitions.append(CharTransition(decLiteral0, False, "0"))
decLiteral.transitions.append(RangeTransition(decLiteral19, False, "1", "9"))
decLiteral19.transitions.append(RangeTransition(decLiteral19, False, "0", "9"))

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== Dot (Separator) ====

AutomataName = "dot"
AllAutomatas = list()

dot = Automata()
dot.insertString(".")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== Semicolon (Operator) ====

AutomataName = "semiColon"
AllAutomatas = list()

semiColon = Automata()
semiColon.insertString(";")

th.write(printAutomata(True))
h.write(printAutomata(False))


# ==== Characteristic (Keyword) ====

AutomataName = "characteristicKw"
AllAutomatas = list()

characteristic = Automata()
characteristic.insertString("characteristic")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== Degree (Keyword) ====

AutomataName = "degreeKw"
AllAutomatas = list()

degree = Automata()
degree.insertString("degree")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== Resource (Keywords) ====

AutomataName = "resourceName"
AllAutomatas = list()

resourceName = Automata()
resourceName.insertString("relation", "Resource::relation")
resourceName.insertString("instance", "Resource::instance")
resourceName.insertString("short_witness", "Resource::shortWitness")

th.write(printAutomata(True, "Resource", "Resource::invalid"))
h.write(printAutomata(False, "Resource", "Resource::invalid"))

# ==== Gate Set (Keyword) ====

AutomataName = "gate_setKw"
AllAutomatas = list()

gate_set = Automata()
gate_set.insertString("gate_set")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== Colon (Operator) ====

AutomataName = "colon"
AllAutomatas = list()

colon = Automata()
colon.insertString(":")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== Gate Set First Words (Keywords) ====

AutomataName = "gate_setFirst"
AllAutomatas = list()

gate_setFirst = Automata()
gate_setFirst.insertString("arithmetic", "GateSetFirst::arithmetic")
gate_setFirst.insertString("boolean", "GateSetFirst::boolean")
gate_setFirst.insertString("@add", "GateSetFirst::add")
gate_setFirst.insertString("@addc", "GateSetFirst::addc")
gate_setFirst.insertString("@mul", "GateSetFirst::mul")
gate_setFirst.insertString("@mulc", "GateSetFirst::mulc")
gate_setFirst.insertString("@and", "GateSetFirst::and_")
gate_setFirst.insertString("@xor", "GateSetFirst::xor_")
gate_setFirst.insertString("@not", "GateSetFirst::not_")

th.write(printAutomata(True, "GateSetFirst", "GateSetFirst::invalid"))
h.write(printAutomata(False, "GateSetFirst", "GateSetFirst::invalid"))

# ==== Comma or SemiColon (symbols) ====

AutomataName = "commaOrSemiColon"
AllAutomatas = list()

commaOrSemiColon = Automata()
commaOrSemiColon.insertString(",", "CommaOrSemiColon::comma")
commaOrSemiColon.insertString(";", "CommaOrSemiColon::semiColon")

th.write(printAutomata(True, "CommaOrSemiColon", "CommaOrSemiColon::invalid"))
h.write(printAutomata(False, "CommaOrSemiColon", "CommaOrSemiColon::invalid"))

# ==== Gate Set Arithmetic Words (Keywords) ====

AutomataName = "gate_setArith"
AllAutomatas = list()

gate_setAriths = Automata()
gate_setAriths.insertString("@add", "GateSetArith::add")
gate_setAriths.insertString("@addc", "GateSetArith::addc")
gate_setAriths.insertString("@mul", "GateSetArith::mul")
gate_setAriths.insertString("@mulc", "GateSetArith::mulc")

th.write(printAutomata(True, "GateSetArith", "GateSetArith::invalid"))
h.write(printAutomata(False, "GateSetArith", "GateSetArith::invalid"))

# ==== Gate Set Boolean Words (Keywords) ====

AutomataName = "gate_setBool"
AllAutomatas = list()

gate_setBools = Automata()
gate_setBools.insertString("@and", "GateSetBool::and_")
gate_setBools.insertString("@xor", "GateSetBool::xor_")
gate_setBools.insertString("@not", "GateSetBool::not_")

th.write(printAutomata(True, "GateSetBool", "GateSetBool::invalid"))
h.write(printAutomata(False, "GateSetBool", "GateSetBool::invalid"))

# ==== Features (Keyword) ====

AutomataName = "featuresKw"
AllAutomatas = list()

features = Automata()
features.insertString("features")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== Features First Words (Keywords) ====

AutomataName = "featureFirst"
AllAutomatas = list()

featuresFirst = Automata()
featuresFirst.insertString("simple", "FeatureFirst::simple")
featuresFirst.insertString("@function", "FeatureFirst::function")
featuresFirst.insertString("@for", "FeatureFirst::for_loop")
featuresFirst.insertString("@switch", "FeatureFirst::switch_case")

th.write(printAutomata(True, "FeatureFirst", "FeatureFirst::invalid"))
h.write(printAutomata(False, "FeatureFirst", "FeatureFirst::invalid"))

# ==== Features Rest Words (Keywords) ====

AutomataName = "featureRest"
AllAutomatas = list()

featuresRest = Automata()
featuresRest.insertString("@function", "FeatureRest::function")
featuresRest.insertString("@for", "FeatureRest::for_loop")
featuresRest.insertString("@switch", "FeatureRest::switch_case")

th.write(printAutomata(True, "FeatureRest", "FeatureRest::invalid"))
h.write(printAutomata(False, "FeatureRest", "FeatureRest::invalid"))

# =================================
# ==== Streaming API Specifics ====
# =================================

# ==== Begin (Keyword) ====

AutomataName = "beginKw"
AllAutomatas = list()

begin = Automata()
begin.insertString("@begin")

th.write(printAutomata(True))
h.write(printAutomata(False))


# ==== Directive Identification ====

AutomataName = "streamingDirective"
AllAutomatas = list()
directive = Automata()
directive.insertString("$", "StreamingDirectiveTypes::gate")
directive.insertString("@delete", "StreamingDirectiveTypes::delete_")
directive.insertString("@assert_zero", "StreamingDirectiveTypes::assert_zero")
directive.insertString("@end", "StreamingDirectiveTypes::end")

th.write(printAutomata(
    True, "StreamingDirectiveTypes", "StreamingDirectiveTypes::invalid"))
h.write(printAutomata(
    False, "StreamingDirectiveTypes", "StreamingDirectiveTypes::invalid"))

# ==== Left Arrow (Operator) ====

AutomataName = "leftArrow"
AllAutomatas = list()

lArrow = Automata()
lArrow.insertString("<-")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== Arithmetic Streaming Directives ====

AutomataName = "arithStreamingDirective"
AllAutomatas = list()
arithDirective = Automata()
arithDirective.insertString("@add", "ArithStreamingDirectives::add")
arithDirective.insertString("@mul", "ArithStreamingDirectives::mul")
arithDirective.insertString("@addc", "ArithStreamingDirectives::addc")
arithDirective.insertString("@mulc", "ArithStreamingDirectives::mulc")
arithDirective.insertString(
    "@short_witness", "ArithStreamingDirectives::witness")
arithDirective.insertString("@instance", "ArithStreamingDirectives::instance")
arithDirective.insertString("<", "ArithStreamingDirectives::assign")
arithDirective.insertString("$", "ArithStreamingDirectives::copy")

th.write(printAutomata(
    True, "ArithStreamingDirectives", "ArithStreamingDirectives::invalid"))
h.write(printAutomata(
    False, "ArithStreamingDirectives", "ArithStreamingDirectives::invalid"))

# ==== Boolean Streaming Directives ====

AutomataName = "boolStreamingDirective"
AllAutomatas = list()
boolDirective = Automata()
boolDirective.insertString("@xor", "BoolStreamingDirectives::xor_")
boolDirective.insertString("@and", "BoolStreamingDirectives::and_")
boolDirective.insertString("@not", "BoolStreamingDirectives::not_")
boolDirective.insertString(
    "@short_witness", "BoolStreamingDirectives::witness")
boolDirective.insertString("@instance", "BoolStreamingDirectives::instance")
boolDirective.insertString("<", "BoolStreamingDirectives::assign")
boolDirective.insertString("$", "BoolStreamingDirectives::copy")

th.write(printAutomata(
    True, "BoolStreamingDirectives", "BoolStreamingDirectives::invalid"))
h.write(printAutomata(
    False, "BoolStreamingDirectives", "BoolStreamingDirectives::invalid"))

# ==== Left Parenthesis (Operator) ====

AutomataName = "leftParen"
AllAutomatas = list()

lParen = Automata()
lParen.insertString("(")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== Wire Number Begin ====

AutomataName = "wireNumberBegin"
AllAutomatas = list()

identBegin = Automata()
identBegin.insertString("$")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== Comma (Separator) ====

AutomataName = "comma"
AllAutomatas = list()

comma = Automata()
comma.insertString(",")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== Field Literal Begin ====

AutomataName = "fieldLiteralBegin"
AllAutomatas = list()

fieldLitBegin = Automata()
fieldLitBegin.insertString("<")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== Field Literal End ====

AutomataName = "fieldLiteralEnd"
AllAutomatas = list()

fieldLitEnd = Automata()
fieldLitEnd.insertString(">")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== Right Parenthesis (Operator) ====

AutomataName = "rightParen"
AllAutomatas = list()

rParen = Automata()
rParen.insertString(")")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== Comma or Right Parenthesis (Conditional Operators) ====

AutomataName = "commaOrRightParen"
AllAutomatas = list()

commaOrCloseParen = Automata()
commaOrCloseParen.insertString(",", "CommaOrRightParen::comma")
commaOrCloseParen.insertString(")", "CommaOrRightParen::rightParen")

th.write(printAutomata(
    True, "CommaOrRightParen", "CommaOrRightParen::invalid"))
h.write(printAutomata(
    False, "CommaOrRightParen", "CommaOrRightParen::invalid"))

# ==== Literal or End (used in instance/short witness streams) ====

AutomataName = "literalOrEnd"
AllAutomatas = list()

literalOrEnd = Automata()
literalOrEnd.insertString("<", "LiteralOrEnd::literal")
literalOrEnd.insertString("@end", "LiteralOrEnd::end")

th.write(printAutomata(True, "LiteralOrEnd", "LiteralOrEnd::invalid"))
h.write(printAutomata(False, "LiteralOrEnd", "LiteralOrEnd::invalid"))

# ============================
# ==== Tree API Specifics ====
# ============================

# ==== Tree Directive Start (global scope) ====

AutomataName = "treeDirectiveGlobalStart"
AllAutomatas = list()

literalOrEnd = Automata()
literalOrEnd.insertString("$", "TreeDirectiveGlobalStart::wireNumber")
literalOrEnd.insertString(
    "@assert_zero", "TreeDirectiveGlobalStart::assertZero")
literalOrEnd.insertString("@delete", "TreeDirectiveGlobalStart::delete_")
literalOrEnd.insertString("@call", "TreeDirectiveGlobalStart::call")
literalOrEnd.insertString("@anon_call", "TreeDirectiveGlobalStart::anonCall")
literalOrEnd.insertString("@for", "TreeDirectiveGlobalStart::forLoop")
literalOrEnd.insertString(
    "@switch", "TreeDirectiveGlobalStart::switchStatement")
literalOrEnd.insertString("@end", "TreeDirectiveGlobalStart::end")
literalOrEnd.insertString("@function", "TreeDirectiveGlobalStart::function")

th.write(printAutomata(
    True, "TreeDirectiveGlobalStart", "TreeDirectiveGlobalStart::invalid"))
h.write(printAutomata(
    False, "TreeDirectiveGlobalStart", "TreeDirectiveGlobalStart::invalid"))

# ==== Identifier ====

def name(automata, rval):
  indexVar = Automata()
  indexVar.setAccept(rval)

  automata.transitions.append(RangeTransition(indexVar, False, 'a', 'z'))
  automata.transitions.append(RangeTransition(indexVar, False, 'A', 'Z'))
  automata.transitions.append(CharTransition(indexVar, False, '_'))

  indexVar.transitions.append(RangeTransition(indexVar, False, 'a', 'z'))
  indexVar.transitions.append(RangeTransition(indexVar, False, 'A', 'Z'))
  indexVar.transitions.append(RangeTransition(indexVar, False, '0', '9'))
  indexVar.transitions.append(CharTransition(indexVar, False, '_'))

  doubleColon = Automata()
  indexVar.transitions.append(CharTransition(doubleColon, False, ':'))
  doubleColon.transitions.append(CharTransition(automata, False, ':'))

  indexVar.transitions.append(CharTransition(automata, False, '.'))

AutomataName = "identifier"
AllAutomatas = list()

identifier = Automata()
name(identifier, "true")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== out (keyword) ====
AutomataName = "outKw"
AllAutomatas = list()

literalOrEnd = Automata()
literalOrEnd.insertString("@out")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== in (keyword) ====
AutomataName = "inKw"
AllAutomatas = list()

literalOrEnd = Automata()
literalOrEnd.insertString("@in")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== instance (keyword) ====
AutomataName = "instanceKw"
AllAutomatas = list()

literalOrEnd = Automata()
literalOrEnd.insertString("@instance")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== short witness (keyword) ====
AutomataName = "shortWitnessKw"
AllAutomatas = list()

literalOrEnd = Automata()
literalOrEnd.insertString("@short_witness")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== Tree Directive Start (non-global scope) ====

AutomataName = "treeDirectiveStart"
AllAutomatas = list()

literalOrEnd = Automata()
literalOrEnd.insertString("$", "TreeDirectiveStart::wireNumber")
literalOrEnd.insertString(
    "@assert_zero", "TreeDirectiveStart::assertZero")
literalOrEnd.insertString("@delete", "TreeDirectiveStart::delete_")
literalOrEnd.insertString("@call", "TreeDirectiveStart::call")
literalOrEnd.insertString("@anon_call", "TreeDirectiveStart::anonCall")
literalOrEnd.insertString("@for", "TreeDirectiveStart::forLoop")
literalOrEnd.insertString(
    "@switch", "TreeDirectiveStart::switchStatement")
literalOrEnd.insertString("@end", "TreeDirectiveStart::end")

th.write(printAutomata(
    True, "TreeDirectiveStart", "TreeDirectiveStart::invalid"))
h.write(printAutomata(
    False, "TreeDirectiveStart", "TreeDirectiveStart::invalid"))

# ==== Directives with a Single Output Wire ====

AutomataName = "singleOutputDirective"
AllAutomatas = list()

listRangeOrArrow = Automata()
listRangeOrArrow.insertString("@add", "SingleOutputDirective::add")
listRangeOrArrow.insertString("@mul", "SingleOutputDirective::mul")
listRangeOrArrow.insertString("@and", "SingleOutputDirective::and_")
listRangeOrArrow.insertString("@xor", "SingleOutputDirective::xor_")
listRangeOrArrow.insertString("@addc", "SingleOutputDirective::addc")
listRangeOrArrow.insertString("@mulc", "SingleOutputDirective::mulc")
listRangeOrArrow.insertString("@not", "SingleOutputDirective::not_")
listRangeOrArrow.insertString(
    "@short_witness", "SingleOutputDirective::witness")
listRangeOrArrow.insertString("@instance", "SingleOutputDirective::instance")
listRangeOrArrow.insertString("<", "SingleOutputDirective::assign")
listRangeOrArrow.insertString("$", "SingleOutputDirective::copy")
listRangeOrArrow.insertString("@call", "SingleOutputDirective::call")
listRangeOrArrow.insertString("@anon_call", "SingleOutputDirective::anonCall")
listRangeOrArrow.insertString("@for", "SingleOutputDirective::forLoop")
listRangeOrArrow.insertString(
    "@switch", "SingleOutputDirective::switchStatement")

th.write(printAutomata(
    True, "SingleOutputDirective", "SingleOutputDirective::invalid"))
h.write(printAutomata(
    False, "SingleOutputDirective", "SingleOutputDirective::invalid"))

# ==== Wire List or Range or Left Arrow ====

AutomataName = "wireListSeparatorsA"
AllAutomatas = list()

listRangeOrArrow = Automata()
listRangeOrArrow.insertString("<-", "WireListSeparators::end")
listRangeOrArrow.insertString(",", "WireListSeparators::list")
listRangeOrArrow.insertString("...", "WireListSeparators::range")

th.write(printAutomata(
    True, "WireListSeparators", "WireListSeparators::invalid"))
h.write(printAutomata(
    False, "WireListSeparators", "WireListSeparators::invalid"))

# ==== Wire List or Left Arrow ====

AutomataName = "wireListSeparatorsB"
AllAutomatas = list()

listOrArrow = Automata()
listOrArrow.insertString("<-", "WireListSeparators::end")
listOrArrow.insertString(",",  "WireListSeparators::list")

th.write(printAutomata(
    True, "WireListSeparators", "WireListSeparators::invalid"))
h.write(printAutomata(
    False, "WireListSeparators", "WireListSeparators::invalid"))

# ==== Wire List or Close Parenthesis ====

AutomataName = "wireListSeparatorsC"
AllAutomatas = list()

listOrArrow = Automata()
listOrArrow.insertString(")", "WireListSeparators::end")
listOrArrow.insertString(",",  "WireListSeparators::list")

th.write(printAutomata(
    True, "WireListSeparators", "WireListSeparators::invalid"))
h.write(printAutomata(
    False, "WireListSeparators", "WireListSeparators::invalid"))

# ==== Wire List or Range or Close Parenthesis ====

AutomataName = "wireListSeparatorsD"
AllAutomatas = list()

listOrArrow = Automata()
listOrArrow.insertString(")", "WireListSeparators::end")
listOrArrow.insertString(",",  "WireListSeparators::list")
listOrArrow.insertString("...",  "WireListSeparators::range")

th.write(printAutomata(
    True, "WireListSeparators", "WireListSeparators::invalid"))
h.write(printAutomata(
    False, "WireListSeparators", "WireListSeparators::invalid"))

# ==== Wire List or Range ====

AutomataName = "wireListSeparatorsE"
AllAutomatas = list()

listOrArrow = Automata()
listOrArrow.insertString(",",  "WireListSeparators::list")
listOrArrow.insertString("...",  "WireListSeparators::range")

th.write(printAutomata(
    True, "WireListSeparators", "WireListSeparators::invalid"))
h.write(printAutomata(
    False, "WireListSeparators", "WireListSeparators::invalid"))

# ==== Wire Number or Instance ====

AutomataName = "wireNumberOrInstance"
AllAutomatas = list()

wireNumOrIns = Automata()
wireNumOrIns.insertString("$", "WireNumberOrInstance::wireNumber")
wireNumOrIns.insertString("@instance", "WireNumberOrInstance::instance")

th.write(printAutomata(
    True,  "WireNumberOrInstance", "WireNumberOrInstance::invalid"))
h.write(printAutomata(
    False, "WireNumberOrInstance", "WireNumberOrInstance::invalid"))


# ==== Wire Number or Feature ====

AutomataName = "wireNumberOrFunction"
AllAutomatas = list()

wireNumOrFunc = Automata()
wireNumOrFunc.insertString("$", "WireNumberOrFunction::wireNumber")
wireNumOrFunc.insertString("@call", "WireNumberOrFunction::call")
wireNumOrFunc.insertString("@anon_call", "WireNumberOrFunction::anonCall")

th.write(printAutomata(
    True,  "WireNumberOrFunction", "WireNumberOrFunction::invalid"))
h.write(printAutomata(
    False, "WireNumberOrFunction", "WireNumberOrFunction::invalid"))

# ==== Iterator Expression ====

AutomataName = "iterExprBase"
AllAutomatas = list()

iterExprNumeric = dict()
iterExprNumeric["dec"] = "IterExprBase::dec"
iterExprNumeric["hex"] = "IterExprBase::hex"
iterExprNumeric["bin"] = "IterExprBase::bin"
iterExprNumeric["oct"] = "IterExprBase::oct"

iterExprBase = Automata()
numericAutomata(iterExprBase, iterExprNumeric)

name(iterExprBase, "IterExprBase::name");
iterExprBase.insertString("(", "IterExprBase::expr");

th.write(printAutomata(True, "IterExprBase", "IterExprBase::invalid"))
h.write(printAutomata(False, "IterExprBase", "IterExprBase::invalid"))

# ==== Iterator Expression Operations (Operators) ====

AutomataName = "iterExprOp"
AllAutomatas = list()

iterExprOp = Automata()
iterExprOp.insertString("+", "IterExprOp::add")
iterExprOp.insertString("-", "IterExprOp::sub")
iterExprOp.insertString("*", "IterExprOp::mul")
iterExprOp.insertString("/", "IterExprOp::div")

th.write(printAutomata(True, "IterExprOp", "IterExprOp::invalid"))
h.write(printAutomata(False, "IterExprOp", "IterExprOp::invalid"))

# ==== IR1 For Loop First/Last (keywords) ====

AutomataName = "firstKw"
AllAutomatas = list()

fromKw = Automata()
fromKw.insertString("@first");

th.write(printAutomata(True))
h.write(printAutomata(False))

AutomataName = "lastKw"
AllAutomatas = list()

toKw = Automata()
toKw.insertString("@last");

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== Function Type (call or anon call) ====

AutomataName = "functionType"
AllAutomatas = list()

functionType = Automata()
functionType.insertString("@call", "FunctionType::call")
functionType.insertString("@anon_call", "FunctionType::anonCall")

th.write(printAutomata(True, "FunctionType", "FunctionType::invalid"))
h.write(printAutomata(False, "FunctionType", "FunctionType::invalid"))

# ==== Directives with a Multiple Output Wires ====

AutomataName = "multiOutputDirective"
AllAutomatas = list()

listRangeOrArrow = Automata()
listRangeOrArrow.insertString("@call", "MultiOutputDirective::call")
listRangeOrArrow.insertString("@anon_call", "MultiOutputDirective::anonCall")
listRangeOrArrow.insertString("@for", "MultiOutputDirective::forLoop")
listRangeOrArrow.insertString(
    "@switch", "MultiOutputDirective::switchStatement")

th.write(printAutomata(
    True, "MultiOutputDirective", "MultiOutputDirective::invalid"))
h.write(printAutomata(
    False, "MultiOutputDirective", "MultiOutputDirective::invalid"))

# ==== End (Keyword) ====

AutomataName = "endKw"
AllAutomatas = list()

end = Automata()
end.insertString("@end")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== Case (Keyword) ====

AutomataName = "caseKw"
AllAutomatas = list()

caseKw = Automata()
caseKw.insertString("@case")

th.write(printAutomata(True))
h.write(printAutomata(False))

# ==== Case Or End (Keywords) ====

AutomataName = "caseOrEnd"
AllAutomatas = list()

caseOrEnd = Automata()
caseOrEnd.insertString("@case", "CaseOrEnd::case_")
caseOrEnd.insertString("@end", "CaseOrEnd::end")

th.write(printAutomata(True, "CaseOrEnd", "CaseOrEnd::invalid"))
h.write(printAutomata(False, "CaseOrEnd", "CaseOrEnd::invalid"))

# ==== Trailer ====
th.write("} } // namespace wtk::irregular\n")
h.write("\n} } // namespace wtk::irregular\n")

h.write("\n#define LOG_IDENTIFIER \"wtk::irregular\"")
h.write("\n#include <stealth_logging.h>\n")
h.write("\n#include <wtk/irregular/Automatas.t.h>\n")
h.write("\n#define LOG_UNINCLUDE")
h.write("\n#include <stealth_logging.h>\n")
h.write("\n#endif//WIZTOOLKIT_AUTOMATA_H_\n")
