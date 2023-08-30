#! /usr/bin/python3

# Copyright (C) 2020-2022 Stealth Software Technologies, Inc.

import os

# Helper library for generating DFAs
from dfa import *

ih = open("target/generated/wtk/irregular/automatas.i.h", "w")

ih.write("#ifndef WIZTOOLKIT_AUTOMATA_H_\n#define WIZTOOLKIT_AUTOMATA_H_\n\n")
ih.write("#include <cstddef>\n\n")
ih.write("#include <wtk/indexes.h>\n")
ih.write("#include <wtk/utils/hints.h>\n\n")
ih.write("#include <wtk/irregular/AutomataCtx.h>\n\n")

ih.write("#define LOG_IDENTIFIER \"automatas\"\n")
ih.write("#include <stealth_logging.h>\n")

ih.write("namespace wtk {\n")
ih.write("namespace irregular {\n\n")

# ==== Whitespace ====

whitespace = DFA("whitespace")
whitespace.setAcceptEof()
ws = whitespace.root()
ws.setAccept()
ws.character(" ", ws)
ws.character("\\n", ws)
ws.lastTransition().addAction("ctx->lineNum++;")
ws.character("\\r", ws)
ws.character("\\t", ws)

cmmt1 = whitespace.newState()
cmmt2 = whitespace.newState()
cmmt3 = whitespace.newState()
cmmt4 = whitespace.newState()

ws.character("/", cmmt1)
cmmt1.character("/", cmmt2)
cmmt1.character("*", cmmt3)

cmmt2.character("\\n", cmmt2, True)
cmmt2.character("\\n", ws)
cmmt2.lastTransition().addAction("ctx->lineNum++;")

cmmt3.character("*", cmmt3, True)
cmmt3.lastTransition().addAction(
    "if(ctx->buffer[ctx->place] == '\\n') { ctx->lineNum++; }")
cmmt3.character("*", cmmt4)
cmmt4.character("*", cmmt4)
cmmt4.character("/", ws)
cmmt4.character("/", cmmt3, True)
cmmt4.lastTransition().addAction(
    "if(ctx->buffer[ctx->place] == '\\n') { ctx->lineNum++; }")

whitespace.optimize()
ih.write(whitespace.toCpp())

# ==== Numbers ====

def numeric(dfa, root, Number_T, val = "val", rval = "true"):
  numLit = root

  # "0"
  numLit0 = dfa.newState()
  numLit.character("0", numLit0)
  numLit.lastTransition().addAction("*" + val + " = " + Number_T + "(0);")
  numLit0.setAccept(rval)

  # hex
  numLitX = dfa.newState()
  numLitXn = dfa.newState()
  numLitXn.setAccept(rval)

  numLit0.character("x", numLitX)
  numLit0.character("X", numLitX)

  numLitX.range("0", "9", numLitXn)
  numLitX.lastTransition().addAction(
      "*" + val + " = " + Number_T + "(ctx->buffer[ctx->place] - '0');")
  numLitX.range("a", "f", numLitXn)
  numLitX.lastTransition().addAction(
      "*" + val + " = " + Number_T + "(0xA + ctx->buffer[ctx->place] - 'a');")
  numLitX.range("A", "F", numLitXn)
  numLitX.lastTransition().addAction(
      "*" + val + " = " + Number_T + "(0xA + ctx->buffer[ctx->place] - 'A');")

  numLitXn.range("0", "9", numLitXn)
  numLitXn.lastTransition().addAction(
      "*" + val + " = " + Number_T + "(" + Number_T + "(*" + val + " << 4) | " + Number_T + "(0x0 + ctx->buffer[ctx->place] - '0'));")
  numLitXn.range("a", "f", numLitXn)
  numLitXn.lastTransition().addAction(
      "*" + val + " = " + Number_T + "(" + Number_T + "(*" + val + " << 4) | " + Number_T + "(0xA + ctx->buffer[ctx->place] - 'a'));")
  numLitXn.range("A", "F", numLitXn)
  numLitXn.lastTransition().addAction(
      "*" + val + " = " + Number_T + "(" + Number_T + "(*" + val + " << 4) | " + Number_T + "(0xA + ctx->buffer[ctx->place] - 'A'));")

  # oct
  numLitO = dfa.newState()
  numLitOn = dfa.newState()
  numLitOn.setAccept(rval)

  numLit0.character("o", numLitO)

  numLitO.range("0", "7", numLitOn)
  numLitO.lastTransition().addAction(
      "*" + val + " = " + Number_T + "(ctx->buffer[ctx->place] - '0');")
  numLitOn.range("0", "7", numLitOn)
  numLitOn.lastTransition().addAction(
      "*" + val + " = " + Number_T + "(" + Number_T + "(*" + val + " << 3) | " + Number_T + "(ctx->buffer[ctx->place] - '0'));")

  # bin
  numLitB = dfa.newState()
  numLitBn = dfa.newState()
  numLitBn.setAccept(rval)

  numLit0.character("b", numLitB)
  numLit0.character("B", numLitB)

  numLitB.range("0", "1", numLitBn)
  numLitB.lastTransition().addAction(
      "*" + val + " = " + Number_T + "(ctx->buffer[ctx->place] - '0');")
  numLitBn.range("0", "1", numLitBn)
  numLitBn.lastTransition().addAction(
      "*" + val + " = " + Number_T + "(" + Number_T + "(*" + val + " << 1) | " + Number_T + "(ctx->buffer[ctx->place] - '0'));")

  # dec
  numLitDec = dfa.newState()
  numLitDec.setAccept(rval)
  numLit.range("1", "9", numLitDec)
  numLit.lastTransition().addAction(
      "*" + val + " = " + Number_T + "(ctx->buffer[ctx->place] - '0');")
  numLitDec.range("0", "9", numLitDec)
  numLitDec.lastTransition().addAction(
      "*" + val + " = " + Number_T + "(" + Number_T + "(*" + val + " * 10) + " + Number_T + "(ctx->buffer[ctx->place] - '0'));")

number = DFA("number")
number.addTemplate("Number_T")
number.addArgument("Number_T* RESTRICT const val")
numeric(number, number.root(), "Number_T", "val")
number.optimize()
ih.write(number.toCpp())

# ==== Indexes ====

index = DFA("index")
index.addArgument("wire_idx* RESTRICT const val")
dollar = index.newState()
index.root().character("$", dollar)
numeric(index, dollar, "wire_idx", "val")
index.optimize()
ih.write(index.toCpp())

# ==== Type or Wire ====

typeOrWire = DFA("typeOrWire", "TypeOrWire", "invalid")

typeOrWire.addArgument("type_idx* RESTRICT const type")
numeric(typeOrWire, typeOrWire.root(), "type_idx", "type", "type")

typeOrWire.addArgument("wire_idx* RESTRICT const wire")
dollar = typeOrWire.newState()
typeOrWire.root().character("$", dollar)
numeric(typeOrWire, dollar, "wire_idx", "wire", "wire")

typeOrWire.optimize()
ih.write(typeOrWire.toCpp())

# ==== Identifiers ====

def identifierize(dfa, arg, rval):
  id_1 = dfa.newState()
  id_1.setAccept(rval)

  dfa.root().range("a", "z", id_1)
  dfa.root().range("A", "Z", id_1)
  dfa.root().character("_", id_1)

  id_1.range("a", "z", id_1)
  id_1.range("A", "Z", id_1)
  id_1.range("0", "9", id_1)
  id_1.character("_", id_1)

  id_n = dfa.newState()
  id_n.setAccept(rval)
  id_n.range("a", "z", id_n)
  id_n.range("A", "Z", id_n)
  id_n.range("0", "9", id_n)
  id_n.character("_", id_n)

  id_dot = dfa.newState()
  id_1.character(".", id_dot)
  id_n.character(".", id_dot)
  id_dot.range("a", "z", id_n)
  id_dot.range("A", "Z", id_n)
  id_dot.character("_", id_n)

  id_colon1 = dfa.newState()
  id_colon2 = dfa.newState()
  id_1.character(":", id_colon1)
  id_n.character(":", id_colon1)
  id_colon1.character(":", id_colon2)
  id_colon2.range("a", "z", id_n)
  id_colon2.range("A", "Z", id_n)
  id_colon2.character("_", id_n)

  dfa.addFinishAction( \
      arg + "->assign(ctx->buffer + ctx->mark, ctx->place - ctx->mark);", rval)

identifier = DFA("identifier")
identifier.addArgument("std::string* RESTRICT const idnt")
identifierize(identifier, "idnt", "true")

identifier.optimize()
ih.write(identifier.toCpp())

# ==== Version (keyword) ====

version = DFA("versionKw")
version.insertString("version")
version.optimize()
ih.write(version.toCpp())

# ==== Dot (operator) ====

dot = DFA("dotOp")
dot.insertString(".")
dot.optimize()
ih.write(dot.toCpp())

# ==== Dash (operator) ====

dash = DFA("dashOp")
dash.insertString("-")
dash.optimize()
ih.write(dash.toCpp())

# ==== Semi Colon (Operator) ====

semiColon = DFA("semiColonOp")
semiColon.insertString(";")
semiColon.optimize()
ih.write(semiColon.toCpp())

# ==== Dash or Semi Colon (Operators) ====

dashOrSemi = DFA("dashOrSemiColonOps", "DashOrSemiColon", "invalid")
dashOrSemi.insertString("-", "dash")
dashOrSemi.insertString(";", "semiColon")
dashOrSemi.optimize()
ih.write(dashOrSemi.toCpp())

# ==== Resource Type (Keywords) ====

resourceType = DFA("resourceTypeKws", "ResourceType", "invalid")
resourceType.insertString("translation", "translation")
resourceType.insertString("circuit", "circuit")
resourceType.insertString("public_input", "publicIn")
resourceType.insertString("private_input", "privateIn")
resourceType.insertString("configuration", "configuration")
resourceType.optimize()
ih.write(resourceType.toCpp())

# ==== Plugin or Type (Keywords) ====

pluginOrType = DFA("pluginOrType", "PluginOrType", "invalid")
pluginOrType.insertString("@plugin", "plugin")
pluginOrType.insertString("@type", "type")
pluginOrType.optimize()
ih.write(pluginOrType.toCpp())

# ==== Field or Ring or Plugin (Keywords) ====

fieldOrRingOrPlugin = \
    DFA("fieldOrRingOrPluginKws", "FieldOrRingOrPlugin", "invalid")
fieldOrRingOrPlugin.insertString("field", "field")
fieldOrRingOrPlugin.insertString("ring", "ring")
fieldOrRingOrPlugin.insertString("@plugin", "plugin")
fieldOrRingOrPlugin.optimize()
ih.write(fieldOrRingOrPlugin.toCpp())

# ==== Type or Convert or Begin (Keywords) ====

typeOrConvertOrBegin = \
  DFA("typeOrConvertOrBeginKws", "TypeOrConvertOrBegin", "invalid")
typeOrConvertOrBegin.insertString("@type", "type")
typeOrConvertOrBegin.insertString("@convert", "convert")
typeOrConvertOrBegin.insertString("@begin", "begin")
typeOrConvertOrBegin.optimize()
ih.write(typeOrConvertOrBegin.toCpp())

# ==== Type (Keyword) ====

typeKw = DFA("typeKw");
typeKw.insertString("@type");
typeKw.optimize()
ih.write(typeKw.toCpp())

# ==== Field or Ring (Keywords) ====

fieldOrRingKws = DFA("fieldOrRingKws", "FieldOrRingKws", "invalid");
fieldOrRingKws.insertString("field", "field");
fieldOrRingKws.insertString("ring", "ring");
fieldOrRingKws.optimize()
ih.write(fieldOrRingKws.toCpp())

# ==== Convert or Begin (Keywords) ====

convertOrBegin = \
  DFA("convertOrBeginKws", "ConvertOrBegin", "invalid")
convertOrBegin.insertString("@convert", "convert")
convertOrBegin.insertString("@begin", "begin")
convertOrBegin.optimize()
ih.write(convertOrBegin.toCpp())

# ==== Begin (Keyword) ====

beginKw = DFA("beginKw")
beginKw.insertString("@begin")
beginKw.optimize()
ih.write(beginKw.toCpp())

# ==== Colon (Operator) ====

colon = DFA("colonOp")
colon.insertString(":")
colon.optimize()
ih.write(colon.toCpp())

# ==== Left Parenthesis (Operator) ====

lparen = DFA("lparenOp")
lparen.insertString("(")
lparen.optimize()
ih.write(lparen.toCpp())

# ==== Right Parenthesis (Operator) ====

rparen = DFA("rparenOp")
rparen.insertString(")")
rparen.optimize()
ih.write(rparen.toCpp())

# ==== Comma (Operator) ====

comma = DFA("commaOp")
comma.insertString(",")
comma.optimize()
ih.write(comma.toCpp())

# ==== Out (Keyword) ====

outKw = DFA("outKw")
outKw.insertString("@out")
outKw.optimize()
ih.write(outKw.toCpp())

# ==== In (Keyword) ====

inKw = DFA("inKw")
inKw.insertString("@in")
inKw.optimize()
ih.write(inKw.toCpp())

# ==== Top Scope Item Start Start (Mixed) ====

topScopeItemStart = DFA("topScopeItemStart", "TopScopeItemStart", "invalid")

# Gates which start with an output wire (standard gates and call)
topScopeItemStart.addArgument("wire_idx* RESTRICT const index")
dollar = topScopeItemStart.newState()
topScopeItemStart.root().character("$", dollar)
numeric(topScopeItemStart, dollar, "wire_idx", "index", "wireIdx")

# Gates which start with a type index (just convert)
topScopeItemStart.addArgument("type_idx* RESTRICT const out_type")
numeric(topScopeItemStart,
    topScopeItemStart.root(), "type_idx", "out_type", "typeIdx")

# Gates which start with a keyword
topScopeItemStart.insertString("@assert_zero", "assertZero")
topScopeItemStart.insertString("@new", "new_")
topScopeItemStart.insertString("@delete", "delete_")
topScopeItemStart.insertString("@call", "call")
topScopeItemStart.insertString("@function", "function")
topScopeItemStart.insertString("@end", "end")
topScopeItemStart.optimize()
ih.write(topScopeItemStart.toCpp())

# ==== Arrow (Operator) ====

arrow = DFA("arrowOp")
arrow.insertString("<-")
arrow.optimize()
ih.write(arrow.toCpp())

# ==== Standard Gate Operations (Mixed) ====

standardGates = DFA("standardGateOps", "StandardGateOps", "invalid")

# named gates
standardGates.insertString("@add", "add")
standardGates.insertString("@mul", "mul")
standardGates.insertString("@addc", "addc")
standardGates.insertString("@mulc", "mulc")
standardGates.insertString("@public", "public_")
standardGates.insertString("@private", "private_")
standardGates.insertString("@call", "call")

# copy without type
standardGates.addArgument("wire_idx* RESTRICT const copy_wire")
dollar = standardGates.newState()
standardGates.root().character("$", dollar)
numeric(standardGates, dollar, "wire_idx", "copy_wire", "copy_wire")

# copy or assign with type
standardGates.addArgument("type_idx* RESTRICT const copy_type")
numeric(standardGates,
    standardGates.root(), "type_idx", "copy_type", "copy_type")

# assign without type
standardGates.insertString("<", "lchevron")

standardGates.optimize()
ih.write(standardGates.toCpp())

# ==== Left Chevron (Operator) ====

lchevron = DFA("lchevronOp")
lchevron.insertString("<")
lchevron.optimize()
ih.write(lchevron.toCpp())

# ==== Right Chevron (Operator) ====

rchevron = DFA("rchevronOp")
rchevron.insertString(">")
rchevron.optimize()
ih.write(rchevron.toCpp())

# ==== Type or Rparen (Mixed) ====

typeOrRparen = DFA("typeOrRparen", "TypeOrRparen", "invalid")

# Type
typeOrRparen.addArgument("type_idx* RESTRICT const type")
numeric(typeOrRparen, typeOrRparen.root(), "type_idx", "type", "type")

# Rparen
typeOrRparen.insertString(")", "rparen")

typeOrRparen.optimize()
ih.write(typeOrRparen.toCpp())

# ==== Copy Operation (Mixed) ====

copyOp = DFA("copyOp", "CopyOp", "invalid")

# copy
copyOp.addArgument("wire_idx* RESTRICT const left")
dollar = copyOp.newState()
copyOp.root().character("$", dollar)
numeric(copyOp, dollar, "wire_idx", "left", "copy")

# assign
copyOp.insertString("<", "assign")

copyOp.optimize()
ih.write(copyOp.toCpp())

# ==== Ranged List (Operators) ====

# A
rangedListA = DFA("rangedListA", "RangedListA", "invalid")
rangedListA.insertString("<-", "arrow")
rangedListA.insertString("...", "range")
rangedListA.insertString(",", "list")

rangedListA.optimize()
ih.write(rangedListA.toCpp())

# B
rangedListB = DFA("rangedListB", "RangedListB", "invalid")
rangedListB.insertString("<-", "arrow")
rangedListB.insertString(",", "list")

rangedListB.optimize()
ih.write(rangedListB.toCpp())

# C
rangedListC = DFA("rangedListC", "RangedListC", "invalid")
rangedListC.insertString(")", "rparen")
rangedListC.insertString("...", "range")
rangedListC.insertString(",", "list")

rangedListC.optimize()
ih.write(rangedListC.toCpp())

# D
rangedListD = DFA("rangedListD", "RangedListD", "invalid")
rangedListD.insertString(")", "rparen")
rangedListD.insertString("...", "range")

rangedListD.optimize()
ih.write(rangedListD.toCpp())

# E
rangedListE = DFA("rangedListE", "RangedListE", "invalid")
rangedListE.insertString(")", "rparen")
rangedListE.insertString(",", "list")

rangedListE.optimize()
ih.write(rangedListE.toCpp())

# F
rangedListF = DFA("rangedListF", "RangedListF", "invalid")
rangedListF.insertString("@out", "out")
rangedListF.insertString("@in", "in")

rangedListF.optimize()
ih.write(rangedListF.toCpp())

# G
rangedListG = DFA("rangedListG", "RangedListG", "invalid")
rangedListG.insertString("@in", "in")

rangedListG.addArgument("type_idx* RESTRICT const type")
numeric(rangedListG, rangedListG.root(), "type_idx", "type", "type")

rangedListG.optimize()
ih.write(rangedListG.toCpp())

# H
rangedListH = DFA("rangedListH", "RangedListH", "invalid")
rangedListH.insertString("<-", "arrow")
rangedListH.insertString("...", "range")

rangedListH.optimize()
ih.write(rangedListH.toCpp())

# I
rangedListI = DFA("rangedListI", "RangedListI", "invalid")
rangedListI.insertString("@private", "private_count")
rangedListI.insertString("@public", "public_count")

rangedListI.addTemplate("Number_T")
rangedListI.addArgument("Number_T* RESTRICT const number")
numeric(rangedListI, rangedListI.root(), "Number_T", "number", "number")

rangedListI.addArgument("std::string* RESTRICT const ident")
identifierize(rangedListI, "ident", "identifier")

rangedListI.optimize()
ih.write(rangedListI.toCpp())

# J
rangedListJ = DFA("rangedListJ", "RangedListJ", "invalid")
rangedListJ.insertString("@public", "public_count")

rangedListJ.addArgument("type_idx* RESTRICT const type")
numeric(rangedListJ, rangedListJ.root(), "type_idx", "type", "type")

rangedListJ.optimize()
ih.write(rangedListJ.toCpp())

# K
rangedListK = DFA("rangedListK", "RangedListK", "invalid")
rangedListK.insertString("...", "range")
rangedListK.insertString(",", "list")
rangedListK.insertString(";", "end")

rangedListK.optimize()
ih.write(rangedListK.toCpp())

# L
rangedListL = DFA("rangedListL", "RangedListL", "invalid")
rangedListL.insertString(",", "list")
rangedListL.insertString(";", "end")

rangedListL.optimize()
ih.write(rangedListL.toCpp())

# ==== Convert (Keyword) ====

convert = DFA("convertKw")
convert.insertString("@convert")

convert.optimize()
ih.write(convert.toCpp())

# ==== Modulus and NoModulus (keywords) ====

modOrNoMod = DFA("modulusKws", "ModulusKws", "invalid")
modOrNoMod.insertString("@no_modulus", "no_modulus")
modOrNoMod.insertString("@modulus", "modulus")

modOrNoMod.optimize()
ih.write(modOrNoMod.toCpp())

# ==== Call (Keyword) ====

callKw = DFA("callKw")
callKw.insertString("@call")

callKw.optimize()
ih.write(callKw.toCpp())

# ==== Call or Private or Public or Copy (keywords) ====

rangeOutDirectives = DFA("rangeOutDirectives", "RangeOutDirectives", "invalid")
rangeOutDirectives.addArgument("wtk::type_idx* t")
rangeOutDirectives.addArgument("wtk::wire_idx* w")
rangeOutDirectives.insertString("@call", "call")
rangeOutDirectives.insertString("@public", "public_")
rangeOutDirectives.insertString("@private", "private_")
wire = rangeOutDirectives.newState()
rangeOutDirectives.root().character("$", wire)
numeric(rangeOutDirectives, rangeOutDirectives.root(), \
    "wtk::type_idx", "t", "type")
numeric(rangeOutDirectives, wire, "wtk::wire_idx", "w", "wire")

rangeOutDirectives.optimize()
ih.write(rangeOutDirectives.toCpp())


# ==== Range (Operator) ====

rangeOp = DFA("rangeOp")
rangeOp.insertString("...")

rangeOp.optimize()
ih.write(rangeOp.toCpp())

# ==== Function Scope First Item Start (Mixed) ====

funcFirstStart = DFA( \
    "funcScopeFirstItemStart", "FuncScopeFirstItemStart", "invalid")

# Standard gates
funcFirstStart.addArgument("wire_idx* RESTRICT const wireIdx")
dollar = funcFirstStart.newState()
funcFirstStart.root().character("$", dollar)
numeric(funcFirstStart, dollar, "wire_idx", "wireIdx", "wireIdx")

# Compound gates
funcFirstStart.addArgument("type_idx* RESTRICT const typeIdx")
numeric(funcFirstStart, funcFirstStart.root(), \
    "type_idx", "typeIdx", "typeIdx")

# Keyword starts
funcFirstStart.insertString("@assert_zero", "assertZero")
funcFirstStart.insertString("@new", "new_")
funcFirstStart.insertString("@delete", "delete_")
funcFirstStart.insertString("@call", "call")
funcFirstStart.insertString("@end", "end")
funcFirstStart.insertString("@plugin", "plugin")


funcFirstStart.optimize()
ih.write(funcFirstStart.toCpp())

# ==== Function Scope First Item Start (Mixed) ====

funcItemStart = DFA("funcScopeItemStart", "FuncScopeItemStart", "invalid")

# Standard gates
funcItemStart.addArgument("wire_idx* RESTRICT const wireIdx")
dollar = funcItemStart.newState()
funcItemStart.root().character("$", dollar)
numeric(funcItemStart, dollar, "wire_idx", "wireIdx", "wireIdx")

# Compound gates
funcItemStart.addArgument("type_idx* RESTRICT const typeIdx")
numeric(funcItemStart, funcItemStart.root(), "type_idx", "typeIdx", "typeIdx")

# Keyword starts
funcItemStart.insertString("@assert_zero", "assertZero")
funcItemStart.insertString("@new", "new_")
funcItemStart.insertString("@delete", "delete_")
funcItemStart.insertString("@call", "call")
funcItemStart.insertString("@end", "end")


funcItemStart.optimize()
ih.write(funcItemStart.toCpp())

# ==== Function Body Gate Start (Mixed) ====

gateStart = DFA("funcGateStart", "FuncGateStart", "invalid")

# Standard gates
gateStart.addArgument("wire_idx* RESTRICT const index")
dollar = gateStart.newState()
gateStart.root().character("$", dollar)
numeric(gateStart, dollar, "wire_idx", "index", "standard")

# Compound gates
gateStart.addArgument("size_t* RESTRICT const o_field")
numeric(gateStart, gateStart.root(), "size_t", "o_field", "compound")

# Keyword starts
gateStart.insertString("@assert_zero", "assertZero")
gateStart.insertString("@new", "new_")
gateStart.insertString("@delete", "delete_")
gateStart.insertString("@call", "call")
gateStart.insertString("@end", "end")

gateStart.optimize()
ih.write(gateStart.toCpp())

# ==== Left Chevron or End (mixed) ====
lchevronOrEnd = DFA("lchevronOrEnd", "LchevronOrEnd", "invalid")
lchevronOrEnd.insertString("<", "lchevron")
lchevronOrEnd.insertString("@end", "end")

lchevronOrEnd.optimize()
ih.write(lchevronOrEnd.toCpp())

# ==== Number then Nullterm (helper for flatbuffer) ====
num_nterm = DFA("numberThenNterm")
num_nterm.addTemplate("Number_T")
num_nterm.addArgument("Number_T* RESTRICT const val")
numeric(num_nterm, num_nterm.root(), "Number_T", "val")

num_nterm_term = num_nterm.newState()
num_nterm_term.setAccept()
for state in num_nterm.states[0:-1]:
  if state.accept:
    state.accept = False
    state.returnVal = num_nterm.defaultReturn
    state.character("\\0", num_nterm_term)

num_nterm.optimize()
ih.write(num_nterm.toCpp())

# ==== Number then Optional Nullterm (helper for flatbuffer) ====
num_opt_nterm = DFA("numberOptNterm", "NumberOptNterm", "invalid")
num_opt_nterm.addTemplate("Number_T")
num_opt_nterm.addArgument("Number_T* RESTRICT const val")
numeric(num_opt_nterm, num_opt_nterm.root(), "Number_T", "val", "opt")

num_opt_nterm_term = num_opt_nterm.newState()
num_opt_nterm_term.setAccept("nterm")
for state in num_opt_nterm.states[0:-1]:
  if state.accept:
    state.character("\\0", num_opt_nterm_term)

num_opt_nterm.optimize()
ih.write(num_opt_nterm.toCpp())

# ==== Identifier then Nullterm (helper for flatbuffer) ====
identifier_nterm = DFA("identifierThenNterm")
identifier_nterm.addArgument("std::string* RESTRICT const idnt")
identifierize(identifier_nterm, "idnt", "true")

identifier_nterm_term = identifier_nterm.newState()
identifier_nterm_term.setAccept()
for state in identifier_nterm.states[0:-1]:
  if state.accept:
    state.accept = False
    state.returnVal = identifier_nterm.defaultReturn
    state.character("\\0", identifier_nterm_term)

identifier_nterm.finishActions["true"] = []
identifier_nterm.addFinishAction( \
    "idnt->assign(ctx->buffer + ctx->mark, ctx->place - (ctx->mark + 1));", "true")

identifier_nterm.optimize()
ih.write(identifier_nterm.toCpp())

# ==== Identifier or Number, then Nullterm (helper for flatbuffer) ====
id_num_nterm = DFA("idOrNumThenNterm", "IdOrNumThenNterm", "invalid")
id_num_nterm.addTemplate("Number_T")
id_num_nterm.addArgument("std::string* RESTRICT const idnt")
id_num_nterm.addArgument("Number_T* RESTRICT const num")

identifierize(id_num_nterm, "idnt", "identifier")
numeric(id_num_nterm, id_num_nterm.root(), "Number_T", "num", "number")

id_num_nterm_id_nterm = id_num_nterm.newState()
id_num_nterm_id_nterm.setAccept("identifier")
id_num_nterm_num_nterm = id_num_nterm.newState()
id_num_nterm_num_nterm.setAccept("number")
for state in identifier_nterm.states[0:-2]:
  if state.accept:
    state.accept = False
    if state.returnVal == "identifier":
      state.character("\\0", id_num_nterm_id_nterm)
    else:
      state.character("\\0", id_num_nterm_num_nterm)
    state.returnVal = id_num_nterm.defaultReturn

# I dunno why the prior function needs its nullterm'd length adjusted
# but this one doesn't *shrug*

id_num_nterm.optimize()
ih.write(id_num_nterm.toCpp())

# ==== Trailer ====

ih.write("} } // namespace wtk::irregular\n\n")

ih.write("#define LOG_UNINCLUDE\n")
ih.write("#include <stealth_logging.h>\n")

ih.write("#endif//WIZTOOLKIT_AUTOMATA_H_\n")
