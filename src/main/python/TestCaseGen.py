#! /usr/bin/python3

# Copyright (C) 2022 Stealth Software Technologies, Inc.

import sys


class GateWriter:
  def __init__(self, out_file, type_idx, omit_type = False, omit_mem = False):
    self.outFile = out_file
    self.typeIdx = type_idx
    if type_idx != 0 and omit_type:
      print("Cannot omit non-zero type index")
      sys.exit(1)
    self.omitType = omit_type
    self.omitMem = omit_mem

  def new(self, first, last, mandatory=False):
    if mandatory or not self.omitMem:
      if self.omitType:
        self.outFile.write("  @new($" \
            + str(first) + " ... $" + str(last) + ");\n")
      else:
        self.outFile.write("  @new(" + str(self.typeIdx) + ": $" \
            + str(first) + " ... $" + str(last) + ");\n")

  def delete(self, first, last, mandatory=False):
    if mandatory or not self.omitMem:
      if self.omitType:
        self.outFile.write("  @delete($" \
            + str(first) + " ... $" + str(last) + ");\n")
      else:
        self.outFile.write("  @delete(" + str(self.typeIdx) + ": $" \
            + str(first) + " ... $" + str(last) + ");\n")

  def add(self, out, left, right):
    self.outFile.write("  $" + str(out) + " <- @add(")
    if not self.omitType:
      self.outFile.write(str(self.typeIdx) + ": ")
    self.outFile.write("$" + str(left) + ", $" + str(right) + ");\n")

  def mul(self, out, left, right):
    self.outFile.write("  $" + str(out) + " <- @mul(")
    if not self.omitType:
      self.outFile.write(str(self.typeIdx) + ": ")
    self.outFile.write("$" + str(left) + ", $" + str(right) + ");\n")

  def addc(self, out, left, right):
    self.outFile.write("  $" + str(out) + " <- @addc(")
    if not self.omitType:
      self.outFile.write(str(self.typeIdx) + ": ")
    self.outFile.write("$" + str(left) + ", < " + str(right) + " >);\n")

  def mulc(self, out, left, right):
    self.outFile.write("  $" + str(out) + " <- @mulc(")
    if not self.omitType:
      self.outFile.write(str(self.typeIdx) + ": ")
    self.outFile.write("$" + str(left) + ", < " + str(right) + " >);\n")

  def copy(self, out, left):
    self.outFile.write("  $" + str(out) + " <- ")
    if not self.omitType:
      self.outFile.write(str(self.typeIdx) + ": ")
    self.outFile.write("$" + str(left) + ";\n")

  def assign(self, out, left):
    self.outFile.write("  $" + str(out) + " <- ")
    if not self.omitType:
      self.outFile.write(str(self.typeIdx) + ": ")
    self.outFile.write("<" + str(left) + ">;\n")

  def public(self, out):
    self.outFile.write("  $" + str(out) + " <- @public(")
    if not self.omitType:
      self.outFile.write(str(self.typeIdx))
    self.outFile.write(");\n")

  def private(self, out):
    self.outFile.write("  $" + str(out) + " <- @private(")
    if not self.omitType:
      self.outFile.write(str(self.typeIdx))
    self.outFile.write(");\n")

  def assertZero(self, left):
    self.outFile.write("  @assert_zero(")
    if not self.omitType:
      self.outFile.write(str(self.typeIdx) + ": ")
    self.outFile.write("$" + str(left) + ");\n")

  def beginFunction(self, name, outputs, inputs):
    self.outFile.write("@function(" + name)

    if len(outputs) > 0:
      self.outFile.write(", @out: ")
      comma = ""
      for output in outputs:
        self.outFile.write(comma + str(output[0]) + ":" + str(output[1]))
        comma = ", "

    if len(inputs) > 0:
      self.outFile.write(", @in: ")
      comma = ""
      for inp in inputs:
        self.outFile.write(comma + str(inp[0]) + ":" + str(inp[1]))
        comma = ", "

    self.outFile.write(")\n")

  def end(self):
    self.outFile.write("@end\n\n")

  def call(self, name, outputs, inputs):
    self.outFile.write("  ")
    if len(outputs) > 0:
      comma = ""
      for output in outputs:
        if len(output) == 1 or output[0] == output[1]:
          self.outFile.write(comma + "$" + str(output[0]))
        else:
          self.outFile.write(comma + "$" + str(output[0]) + " ... $" \
              + str(output[1]))
        comma = ", "
      self.outFile.write(" <- ")

    self.outFile.write("@call(" + name)

    if len(inputs) > 0:
      comma = ", "
      for inp in inputs:
        if len(inp) == 1 or inp[0] == inp[1]:
          self.outFile.write(comma + "$" + str(inp[0]))
        else:
          self.outFile.write(comma + "$" + str(inp[0]) + " ... $" \
              + str(inp[1]))
        comma = ", "

    self.outFile.write(");\n")

  def comment(self, cmt):
    self.outFile.write("  // " + cmt + "\n")

  def newLine(self):
    self.outFile.write("\n")

  def pluginBinding(self, name, operation, parameters):
    self.outFile.write("  @plugin(" + name + ", " + operation)
    for parameter in parameters:
      self.outFile.write(", " + str(parameter))
    self.outFile.write(");\n")

def convert2(out_type, out_wire, in_type, in_wire):
  out_type.outFile.write("  " + str(out_type.typeIdx) + ": $" \
      + str(out_wire) + " <- @convert(" + str(in_type.typeIdx) + ": $" \
      + str(in_wire) + ");\n")

def convert4(out_type, out_first, out_last, in_type, in_first, in_last):
  out_type.outFile.write("  " + str(out_type.typeIdx) + ": $" \
      + str(out_first) + " ... $" + str(out_last) + " <- @convert(" \
      + str(in_type.typeIdx) + ": $" + str(in_first) + " ... $" + \
      str(in_last) + ");\n")
