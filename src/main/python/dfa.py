#! /usr/bin/python3

# Copyright (C) 2020-2022 Stealth Software Technologies, Inc.

# This is a python library for generating C++ code which evaluates a DFA.
# A state-machine graph is built in python memory, and then serialized into
# a C++ function which has a loop and a bunch of if/else/case statements which
# matches text against the DFA.
#
# Each DFA generated is represented by an (internal) enum of states, along with
# a (public) function which incodes the state transitions.
#
# NOTE: Certain lines of the generated C++ use bitwise operations in place of
# logical operations, because these avoid short-circuit evaluation. In this
# case, short-circuiting is a bit slower, because the comparisons are so small
# and I presume it just causes expulsion from the branch-predictor. When using
# bitwise operations, the code-generator is careful to insert additional
# parenthesis to avoid operator precedence issues.

#* The DFA class is the entry-point to a DFA graph.
class DFA:

  def __init__(self, n, rtype = "bool", dret = "false"):
    self.name = n
    self.returnType = rtype
    self.defaultReturn = dret
    self.states = [ State(0, self.defaultReturn) ];
    self.acceptEof = False
    self.returnEof = self.defaultReturn
    self.templates = [ ]
    self.arguments = [ ]
    self.variables = [ ]
    self.finishActions = dict()

  def newState(self):
    ret = State(len(self.states), self.defaultReturn);
    self.states.append(ret);
    return ret;

  def root(self):
    return self.states[0]

  def insertString(self, string, acceptVal = "true"):
    curr = self.root()
    i = 0
    for c in string:
      transition = curr.findCharTrans(c)
      if transition is None:
        tmp = self.newState()
        curr.character(c, tmp)
        curr = tmp
      else:
        curr = transition.nextState

      if i == len(string) - 1:
        curr.setAccept(acceptVal)
      i = i + 1

  def setAcceptEof(self, rval = "true"):
    self.acceptEof = True
    self.returnEof = rval

  def numIncomingTransitions(self, key):
    ins = 0
    for state in self.states:
      for transition in state.transitions:
        if transition.nextState is key:
          ins = ins + 1
    return ins

  def printSimple(self):
    print("automata:", self.name, ", type:", self.returnType)
    for state in self.states:
      print("  state: ", state.num)
      for transition in state.transitions:
        if transition.type == "loop":
          print("    short loop:")
          for condition in transition.conditions:
            print("      condition:", condition.simpleCondition())
          for action in transition.actions:
            print("      action: ", action)
        elif transition.preAccept:
          print("    transition:", transition.simpleCondition(), ", pre-accept: ",
              self.returnType + "::" + transition.nextState.returnVal)
          for action in transition.actions:
            print("      action: ", action)
        else:
          print("    transition:", transition.simpleCondition(),
              ", to:", transition.nextState.num)
          for action in transition.actions:
            print("      action:", action)
      if state.returnVal in self.finishActions:
        for action in self.finishActions[state.returnVal]:
          print("    finish action:", action)
      print("    return ", self.returnType, "::",  state.returnVal)

  # Before we start optimizing, make a list of possible return values
  def findReturnEnum(self):
    self.returnEnum = [ self.defaultReturn ]
    for state in self.states:
      if not state.returnVal in self.returnEnum:
        self.returnEnum.append(state.returnVal)

    if self.acceptEof and not self.returnEof in self.returnEnum:
      self.returnEnum.append(self.returnEof)

  # If a transition goes to a non-accepting state with a single transition
  # we can convert it to a "lookahead" to remove the whole state.
  def findLookaheads(self):
    i = 0
    while i < len(self.states):
      state = self.states[i]

      for j in range(0, len(state.transitions)):
        transition = state.transitions[j]
        nextState = transition.nextState

        if transition.type != "lookahead":
          lookaheads = [ transition ]

          # TODO: is the incoming transitions condition falsifiable?
          # 1 >= self.numIncomingTransitions(nextState) \
          while 1 == len(nextState.transitions) \
              and not nextState.accept \
              and not nextState.transitions[0] in lookaheads:
            nextTrans = nextState.transitions[0]

            if nextTrans.type == "lookahead":
              lookaheads.extend(nextTrans.lookaheads)
            else:
              lookaheads.append(nextTrans)

            # If there's no more transitions into a state, remove it.
            if nextState in self.states \
                and 1 >= self.numIncomingTransitions(nextState):
              idx = self.states.index(nextState)
              self.states.remove(nextState)
              if idx <= i:
                i = i - 1

            nextState = nextTrans.nextState
          # if it found any lookaheads, convert the transition to lookahead
          if len(lookaheads) > 1:

            newTrans = Transition(nextState, False)
            newTrans.type = "lookahead"
            newTrans.lookaheads = lookaheads

            state.transitions[j] = newTrans
            transition = newTrans

      i = i + 1

    # if the transition's nextState accepts, and it has no incoming
    # transitions, then it can be "pre-accepted"
    i = 0
    while i < len(self.states):
      state = self.states[i]
      for transition in state.transitions:
        nextState = transition.nextState
        if nextState.accept and 0 == len(nextState.transitions):

            transition.preAccept = True
            # If the next state has no more incoming transitions, remove it
            if nextState in self.states \
                and 1 >= self.numIncomingTransitions(nextState):
              idx = self.states.index(nextState)
              self.states.remove(nextState)
              if idx <= i:
                i = i - 1

      i = i + 1

  # If a transition (or a sequence of transitions) cycle right back to the
  # same state, then bunch them into a loop.
  def findShortLoops(self):
    for state in self.states:
      i = 0
      while i < len(state.transitions):
        transition = state.transitions[i]
        # TODO: for now short-looping on lookaheads is... difficult
        # so we're not gonna do it
        if transition.nextState == state and transition.type != "lookahead":
          loops = [ transition ]
          actions = [ ]
          for action in transition.actions:
            actions.append( [ transition.condition(), action ] )

          j = 1
          while i + j < len(state.transitions):
            candidate = state.transitions[i + j]

            if candidate.nextState == state and candidate.type != "lookahead":
              loops.append(candidate)
              state.transitions.remove(candidate)
              for action in candidate.actions:
                actions.append( [ candidate.condition(), action ] )

            elif candidate.overlapsAny(loops):
              j = len(state.transitions)
            else:
              j = j + 1

          newTrans = Transition(state, False)
          newTrans.type = "loop"
          newTrans.conditions = loops
          newTrans.actions = [ ]
          for action in actions:
            if len(loops) == 1:
              newTrans.actions.append(action[1])
            else:
              newTrans.actions.append(
                  "if" + action[0] + " { " + action[1] + " }")
          state.transitions[i] = newTrans
        i = i + 1

  def optimize(self):
    self.findReturnEnum()
    self.findLookaheads()
    self.findShortLoops()

  def toCppReturnVal(self, val):
    if self.returnType == "bool":
      return val
    else:
      return self.returnType + "::" + val

  def toCppFinishActions(self, space, rval):
    ret = ""
    if rval in self.finishActions:
      for action in self.finishActions[rval]:
        ret += space + action + "\n"
    ret += space + "return " + self.toCppReturnVal(rval) + ";\n"
    return ret

  def toCppStateName(self, state):
    return self.name + "States::state" + str(state.num)

  def toCppTransitionUpdate(self, transition):
    ret = ""
    if(transition.type == "lookahead"):
      advance = 0
      for lookahead in transition.lookaheads:
        if len(lookahead.actions) == 0:
          advance += 1
        else:
          ret += "          ctx->place += " + str(advance) + ";\n"
          for action in lookahead.actions:
            ret += "          " + action + "\n"
          advance = 1

      ret += "          ctx->place += " + str(advance) + ";\n"

    else:
      for action in transition.actions:
        ret += "          " + action + "\n"

      ret += "          ctx->place++;\n"

    if transition.preAccept:
      ret += self.toCppFinishActions("          ", transition.nextState.returnVal)
    else:
      ret += "          state = " + self.toCppStateName(transition.nextState) + ";\n" + "          break;\n"

    return ret

  def toCppLoopTransition(self, transition, useRetry):
    ret = ""
    ret += "        do\n        {\n"
    if len(transition.conditions) > 1:
      ret += "          if("
    else:
      ret += "          if"

    conjunction = ""
    for condition in transition.conditions:
      ret += conjunction + condition.condition()
      conjunction = "\n              | "

    if len(transition.conditions) > 1:
      ret += ")\n          {\n"
    else:
      ret += "\n          {\n"

    for action in transition.actions:
      ret += "            " + action + "\n"

    ret += "            ctx->place += 1;\n"
    if useRetry:
      ret += "            retryState = true;\n"
    ret += "          }\n          else\n          {\n"
    ret += "            break;\n"
    ret += "          }\n"
    ret += "        } while(LIKELY(ctx->place <= ctx->last));\n"
    return ret


  def toCpp(self):
    ret = ""

    if self.returnType != "bool":
      ret += "enum class " + self.returnType + "\n{\n  "
      comma = ""

      for val in self.returnEnum:
        ret += comma + val
        comma = ",\n  "

      ret += "\n};\n\n"

    ret += "enum class " + self.name + "States" + "\n{\n  "
    comma = ""

    for state in self.states:
      ret += comma + "state" + str(state.num)
      comma = ",\n  "

    ret += "\n};\n\n"

    if len(self.templates) > 0:
      ret += "template<"
      conjunction = ""
      for template in self.templates:
        ret += conjunction + "typename " + template
        conjunction = ", "
      ret += ">\n"

    ret += "ALWAYS_INLINE static inline " + self.returnType + " " + self.name + "(AutomataCtx* RESTRICT const ctx"

    for argument in self.arguments:
      ret += ", " + argument
    ret += ")\n{\n"

    for var in self.variables:
      ret += "  " + var + ";\n"

    ret += "  ctx->mark = ctx->place;\n"
    ret += "  " + self.name + "States state = " + self.name + "States::state0;\n\n"

    longestLookahead = 0
    for s in self.states:
      for t in s.transitions:
        if t.type == "lookahead" and len(t.lookaheads) > longestLookahead:
          longestLookahead = len(t.lookaheads)

    ret += "  do\n  {\n"
    ret += "    while(LIKELY(LIKELY(ctx->place + " + str(longestLookahead) + " <= ctx->last)\n        || LIKELY(ctx->eof & (ctx->place <= ctx->last))))\n    {\n"
    ret += "      switch(state)\n      {\n"

    for state in self.states:
      ret += "      case " + self.toCppStateName(state) + ":\n"
      ret += "      {\n"

      loopCount = 0
      first = True
      useRetry = False
      for i in range(0, len(state.transitions)):
        transition = state.transitions[i]

        if transition.type == "loop":
          if not first and loopCount == 0:
            ret += "        bool retryState = false;\n"
            useRetry = True
          loopCount += 1

          ret += self.toCppLoopTransition(transition, useRetry)

          checkDist = 0
          for t in state.transitions[i + 1:]:
            if t.type == "loop":
              break
            elif t.type == "lookahead" and checkDist < len(t.lookaheads):
              checkDist = len(t.lookaheads) - 1

          if checkDist == 0:
            ret += "        if(UNLIKELY(ctx->last < ctx->place)) { break; }\n\n"
          else:
            ret += "        if(UNLIKELY(UNLIKELY((LIKELY(!ctx->eof) && UNLIKELY(ctx->last < ctx->place + " + str(checkDist) + "))) || ctx->last < ctx->place)) { break; }\n\n"

        else:
          ret += "        if" + transition.condition() + "\n        {\n"
          ret += self.toCppTransitionUpdate(transition)
          ret += "        }\n\n"

        first = False

      if useRetry:
        ret += "        if(retryState) { break; }\n\n"
      if not state.accept:
        ret += "        log_error(\"%s:%zu: " + self.name + " could not recognize \\\'%.*s\\\' \\\'%c\\\' \\\'%.*s\\\'\", ctx->name, ctx->lineNum, (int) (ctx->place - ctx->mark), ctx->buffer + ctx->mark, ctx->buffer[ctx->place], (int) ((ctx->last < ctx->place + 9) ? ctx->last - ctx->place : 8), ctx->buffer + ctx->place + 1);\n"
      ret += self.toCppFinishActions("        ", state.returnVal)

      ret += "      }\n"

    ret += "      }\n"
    ret += "    }\n"
    ret += "    if(LIKELY(!ctx->eof))\n    {\n"
    ret += "      if(UNLIKELY(!ctx->update()))\n      {\n"
    ret += self.toCppFinishActions("        ", self.defaultReturn)
    ret += "      }\n"
    ret += "    }\n"
    ret += "  } while(LIKELY(ctx->place <= ctx->last));\n\n"
    if self.acceptEof:
      ret += self.toCppFinishActions("  ", self.returnEof)
    else:
      ret += "  log_error(\"%s:%zu: unexpectedly reached end\", ctx->name, ctx->lineNum);\n"
      ret += self.toCppFinishActions("  ", self.defaultReturn)
    ret += "}\n\n"

    return ret

  def addTemplate(self, template):
    self.templates.append(template)

  def addArgument(self, argument):
    self.arguments.append(argument)

  def addVariable(self, variable):
    self.variables.append(variable)

  def addFinishAction(self, action, rval = "true"):
    if not rval in self.finishActions:
      self.finishActions[rval] = []
    self.finishActions[rval].append(action)

class State:

  def __init__(self, n, rval = "false"):
    self.accept = False
    self.returnVal = rval
    self.transitions = list()
    self.num = n

  def character(self, c, nxt, neg = False):
    t = Transition(nxt, neg)
    t.character = c
    t.type = "char"
    self.transitions.append(t)
    return nxt

  def range(self, f, l, nxt, neg = False):
    t = Transition(nxt, neg)
    t.first = f
    t.last = l
    t.type = "range"
    self.transitions.append(t)
    return nxt

  def setAccept(self, aval = "true"):
    self.accept = True
    self.returnVal = aval

  def findCharTrans(self, c):
    for t in self.transitions:
      if t.type == "char" and t.character == c:
        return t
    return None

  def lastTransition(self):
    return self.transitions[-1]

class Transition:

  def __init__(self, ns, neg):
    self.nextState = ns
    self.negate = neg
    self.type = "invalid"
    self.preAccept = False
    self.actions = []

  def simpleCondition(self):
    neg = "not" if self.negate else ""
    if self.type == "range":
      return neg + "range[" + self.first + ", " + self.last + "]"
    elif self.type == "char":
      return neg + "char(" + self.character + ")"
    elif self.type == "lookahead":
      ret = neg = "lookahead: "
      conjunction = ""
      for transition in self.lookaheads:
        ret += conjunction + transition.simpleCondition()
        conjunction = ", "
      return ret

  def condition(self, place = 0):
    ret = "("
    c = "ctx->buffer[ctx->place + " + str(place) + "]"
    if self.type == "range" and not self.negate:
      ret += "(" + c + " >= \'" + self.first + "\')"
      ret += " & (" + c + " <= \'" + self.last + "\')"

    elif self.type == "range" and self.negate:
      ret += "(" + c + " <= \'" + self.first + "\')"
      ret += " | (" + c + " >= \'" + self.last + "\')"

    elif self.type == "char" and not self.negate:
      ret += c + " == \'" + self.character + "\'"

    elif self.type == "char" and self.negate:
      ret += c + " != \'" + self.character + "\'"

    elif self.type == "lookahead":
      ret += "LIKELY(ctx->last >= ctx->place + " + str(max(len(self.lookaheads) - 1, 0)) + ") && "
      ret += self.lookaheads[0].condition(place) + " && LIKELY("
      place += 1

      conjunction = ""
      for transition in self.lookaheads[1:]:
        ret += conjunction + transition.condition(place)
        conjunction = " & "
        place += 1

      ret += ")"

    ret += ")"
    return ret;

  # TODO: checks if this and another transitions' conditions overlap
  def overlaps(self, other):
    return True

  # Checks if this transition's condition overlaps with any of the others
  def overlapsAny(self, others):
    for other in others:
      if self.overlaps(other):
        return True
    return False

  def addAction(self, action):
    self.actions.append(action)
