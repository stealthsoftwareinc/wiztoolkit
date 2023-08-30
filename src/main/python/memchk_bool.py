#! /usr/bin/python3

# Copyright (C) 2023, Stealth Software Technologies, Inc.

# This script will generate an IR statement for checking that every cell
# in an IR Boolean RAM buffer works as intended. It does so by writing various
# values to the buffer, and checking their correct rememberance.

import sys
import random
import TestCaseGen as tcg

def streamHeader(f, res):
  f.write("version 2.1.0;\n")
  f.write(res + ";\n")
  f.write("@type field 2;\n")
  f.write("@begin\n")

def relationHeader(f, res, size, idx_bits, elt_bits):
  f.write("version 2.1.0;\n")
  f.write(res + ";\n")
  f.write("@plugin ram_bool_v0;\n")
  f.write("@plugin iter_v0;\n")
  f.write("@type field 2;\n")
  f.write("@type @plugin(ram_bool_v0, ram, 0, " \
          + str(idx_bits) + ", " + str(elt_bits) + ", 1, " + str(size) \
          + ", " + str(size) + ");\n")
  f.write("@begin\n")

def makeMemchk(r, size, idx_bits, elt_bits):
  r.beginFunction("ram_init", [[1, 1]], [[0, elt_bits]])
  r.pluginBinding("ram_bool_v0", "init", [size])
  r.newLine()

  r.beginFunction("ram_read", [[0, elt_bits]], [[1, 1], [0, idx_bits]])
  r.pluginBinding("ram_bool_v0", "read", [])
  r.newLine()

  r.beginFunction("ram_write", [], [[1, 1], [0, idx_bits], [0, elt_bits]])
  r.pluginBinding("ram_bool_v0", "write", [])
  r.newLine()

  curr_range = [0, elt_bits - 1]
  r.new(curr_range[0], curr_range[1])
  for i in range(0, elt_bits):
    r.assign(i, 1)
  r.call("ram_init", [[0]], [curr_range])
  r.newLine()

  r.beginFunction("expect_1s", [], [[1, 1], [0, idx_bits]])
  r.call("ram_read", [[idx_bits, idx_bits + elt_bits - 1]], [[0], [0, idx_bits - 1]])
  for i in range(idx_bits, idx_bits + elt_bits):
    r.addc(i + elt_bits, i, 1)
    r.assertZero(i + elt_bits)
  r.end()

  r.beginFunction("iter_expect_1s", [], [[1, 1]])
  r.pluginBinding("iter_v0", "map_enumerated", ["expect_1s", 1, size])
  r.newLine()

  r.call("iter_expect_1s", [], [[0]])

  r.beginFunction("write_0s", [], [[1, 1], [0, idx_bits]])
  r.new(idx_bits, idx_bits + elt_bits - 1)
  for i in range(idx_bits, idx_bits + elt_bits):
    r.assign(i, 0)
  r.call("ram_write", [], [[0], [0, idx_bits - 1], [idx_bits, idx_bits + elt_bits - 1]])
  r.end()

  r.beginFunction("iter_write_0s", [], [[1, 1]])
  r.pluginBinding("iter_v0", "map_enumerated", ["write_0s", 1, size])
  r.newLine()

  r.call("iter_write_0s", [], [[0]])

  r.beginFunction("expect_0s", [], [[1, 1], [0, idx_bits]])
  r.call("ram_read", [[idx_bits, idx_bits + elt_bits - 1]], [[0], [0, idx_bits - 1]])
  for i in range(idx_bits, idx_bits + elt_bits):
    r.assertZero(i)
  r.end()

  r.beginFunction("iter_expect_0s", [], [[1, 1]])
  r.pluginBinding("iter_v0", "map_enumerated", ["expect_0s", 1, size])
  r.newLine()

  r.call("iter_expect_0s", [], [[0]])

  r.beginFunction("write_idxs", [], [[1, 1], [0, idx_bits]])
  r.new(idx_bits, idx_bits + elt_bits - 1)
  if idx_bits < elt_bits:
    for i in range(idx_bits, elt_bits):
      r.assign(i, 0)
  for i in range(0, idx_bits):
    if elt_bits - idx_bits + i >= 0:
      r.copy(idx_bits + elt_bits - idx_bits + i, i)
  r.call("ram_write", [], [[0], [0, idx_bits - 1], [idx_bits, idx_bits + elt_bits - 1]])
  r.end()

  r.beginFunction("iter_write_idxs", [], [[1, 1]])
  r.pluginBinding("iter_v0", "map_enumerated", ["write_idxs", 1, size])

  r.call("iter_write_idxs", [], [[0]])

  r.beginFunction("expect_idxs", [], [[1, 1], [0, idx_bits]])
  r.call("ram_read", [[idx_bits, idx_bits + elt_bits - 1]], [[0], [0, idx_bits - 1]])
  if idx_bits < elt_bits:
    for i in range(idx_bits, elt_bits):
      r.assertZero(i)
  for i in range(0, idx_bits):
    if elt_bits - idx_bits + i >= 0:
      r.add(idx_bits + elt_bits + i, idx_bits + elt_bits - idx_bits + i, i)
      r.assertZero(idx_bits + elt_bits + i)
  r.end()

  r.beginFunction("iter_expect_idxs", [], [[1, 1]])
  r.pluginBinding("iter_v0", "map_enumerated", ["expect_idxs", 1, size])

  r.call("iter_expect_idxs", [], [[0]])

  r.beginFunction("write_vals", [], [[1, 1], [0, elt_bits], [0, idx_bits]])
  r.call("ram_write", [], [[0], [elt_bits, elt_bits + idx_bits - 1], [0, elt_bits - 1]])
  r.end()

  r.beginFunction("iter_write_vals", [], [[1, 1], [0, elt_bits]])
  r.pluginBinding("iter_v0", "map_enumerated", ["write_vals", 2, size])

  r.beginFunction("expect_vals", [], [[1, 1], [0, elt_bits], [0, idx_bits]])
  r.call("ram_read", [[elt_bits + idx_bits, elt_bits * 2 + idx_bits - 1]], [[0], [elt_bits, elt_bits + idx_bits - 1]])
  for i in range(0, elt_bits):
    r.add(elt_bits * 2 + idx_bits + i, elt_bits + idx_bits + i, i)
    r.assertZero(elt_bits * 2 + idx_bits + i)
  r.end()

  r.beginFunction("iter_expect_vals", [], [[1, 1], [0, elt_bits]])
  r.pluginBinding("iter_v0", "map_enumerated", ["expect_vals", 2, size])

  for i in range(0, elt_bits):
    curr_range[0] += elt_bits
    curr_range[1] += elt_bits

    r.new(curr_range[0], curr_range[1])
    for j in range(0, elt_bits):
      r.assign(curr_range[0] + j, 1 if i == j else 0)
    r.call("iter_write_vals", [], [[0], curr_range])
    r.call("iter_expect_vals", [], [[0], curr_range])
    r.delete(curr_range[0], curr_range[1])

  for i in range(0, elt_bits):
    curr_range[0] += elt_bits
    curr_range[1] += elt_bits

    r.new(curr_range[0], curr_range[1])
    for j in range(0, elt_bits):
      r.assign(curr_range[0] + j, 1 if i != j else 0)
    r.call("iter_write_vals", [], [[0], curr_range])
    r.call("iter_expect_vals", [], [[0], curr_range])
    r.delete(curr_range[0], curr_range[1])

  curr_range[0] += elt_bits
  curr_range[1] += elt_bits

  r.new(curr_range[0], curr_range[1])
  for j in range(0, elt_bits):
    r.assign(curr_range[0] + j, 1 if j % 2 == 0 else 0)
  r.call("iter_write_vals", [], [[0], curr_range])
  r.call("iter_expect_vals", [], [[0], curr_range])
  r.delete(curr_range[0], curr_range[1])

  curr_range[0] += elt_bits
  curr_range[1] += elt_bits

  r.new(curr_range[0], curr_range[1])
  for j in range(0, elt_bits):
    r.assign(curr_range[0] + j, 1 if j % 2 == 1 else 0)
  r.call("iter_write_vals", [], [[0], curr_range])
  r.call("iter_expect_vals", [], [[0], curr_range])
  r.delete(curr_range[0], curr_range[1])

  curr_range[0] += elt_bits
  curr_range[1] += elt_bits

  r.new(curr_range[0], curr_range[1])
  for j in range(0, elt_bits):
    r.assign(curr_range[0] + j, 1)
  r.call("iter_write_vals", [], [[0], curr_range])
  r.call("iter_expect_vals", [], [[0], curr_range])
  r.delete(curr_range[0], curr_range[1])

  r.beginFunction("write_witnesses", [], [[1, 1]])
  r.new(0, idx_bits + elt_bits - 1)
  for i in range (0, idx_bits + elt_bits):
    r.private(i)
  r.call("ram_write", [], [[0], [0, idx_bits - 1], [idx_bits, idx_bits + elt_bits - 1]])
  r.end()

  r.beginFunction("iter_write_witnesses", [], [[1, 1]])
  r.pluginBinding("iter_v0", "map", ["write_witnesses", 1, size])

  r.beginFunction("expect_witnesses", [], [[1, 1]])
  r.new(0, idx_bits + elt_bits * 3 - 1)
  for i in range(0, idx_bits):
    r.private(i)
  r.call("ram_read", [[idx_bits, idx_bits + elt_bits - 1]], [[0], [0, idx_bits - 1]])

  for i in range(0, elt_bits):
    r.private(idx_bits + elt_bits + i)
    r.add(idx_bits + elt_bits * 2 + i, idx_bits + elt_bits + i, idx_bits + i)
    r.assertZero(idx_bits + elt_bits * 2 + i)
  r.delete(0, idx_bits + elt_bits * 3 - 1)
  r.end()

  r.beginFunction("iter_expect_witnesses", [], [[1, 1]])
  r.pluginBinding("iter_v0", "map", ["expect_witnesses", 1, size])

  r.call("iter_write_witnesses", [], [[0]])
  r.call("iter_expect_witnesses", [], [[0]])
  r.call("iter_expect_witnesses", [], [[0]])
  r.call("iter_expect_witnesses", [], [[0]])

  r.beginFunction("iter_ram_read", [[0, elt_bits * size]], [[1, 1]])
  r.pluginBinding("iter_v0", "map_enumerated", ["ram_read", 1, size])

  curr_range[0] += elt_bits
  curr_range[1] = curr_range[0] + elt_bits * size - 1
  r.call("iter_ram_read", [curr_range], [[0]])

  r.beginFunction("expect_witness_noram", [], [[0, elt_bits]])
  for i in range(0, elt_bits):
    r.private(elt_bits + i)
    r.add(elt_bits * 2 + i, i, elt_bits + i)
    r.assertZero(elt_bits * 2 + i)
  r.end()

  r.beginFunction("iter_expect_witness_noram", [], [[0, elt_bits * size]])
  r.pluginBinding("iter_v0", "map", ["expect_witness_noram", 0, size])

  r.call("iter_expect_witness_noram", [], [curr_range])

  r.end()

def decomposeWitness(f, val, bits):
  f.write("// decomp bits: " + str(bits) + ", value: " + str(val) + "\n")
  decomp = [0] * bits
  for i in range(1, bits + 1):
    decomp[bits - i] = val & 1
    val = val >> 1
  for i in range(0, bits):
    f.write("<" + str(decomp[i]) + ">;\n")

def makeWitness(f, size, idx_bits, elt_bits):
  bfr = [(1 << elt_bits) - 1] * size
  acs_ptrn = []
  for i in range(0, size):
    idx = random.randrange(0, size)
    val = random.randrange(0, 1 << elt_bits)
    decomposeWitness(f, int(idx), idx_bits)
    decomposeWitness(f, int(val), elt_bits)
    bfr[idx] = val
    acs_ptrn.append(idx)
  for i in range(0, size):
    decomposeWitness(f, int(acs_ptrn[i]), idx_bits)
    decomposeWitness(f, int(bfr[acs_ptrn[i]]), elt_bits)
  for i in range(0, size):
    idx = random.randrange(0, size)
    decomposeWitness(f, int(idx), idx_bits)
    decomposeWitness(f, int(bfr[idx]), elt_bits)
  for i in range(0, size):
    decomposeWitness(f, int(i), idx_bits)
    decomposeWitness(f, int(bfr[i]), elt_bits)
  for i in range(0, size):
    decomposeWitness(f, bfr[i], elt_bits)
  f.write("@end\n")

def makeMemChkIR0(size, idx_bits, elt_bits, rel_file, ins_file, wit_file):
  relationHeader(rel_file, "circuit", size, idx_bits, elt_bits)
  streamHeader(ins_file, "public_input")
  streamHeader(wit_file, "private_input")
  ins_file.write("@end\n")
  makeWitness(wit_file, size, idx_bits, elt_bits)
  makeMemchk(tcg.GateWriter(rel_file, 0), size, idx_bits, elt_bits)

  rel_file.flush()
  rel_file.close()

  ins_file.flush()
  ins_file.close()

  wit_file.flush()
  wit_file.close()

if __name__ == "__main__":
  if len(sys.argv) != 5:
    print("Usage: memchk <size> <index> <element> <output>\n")
    print("Generate a memory test circuit using the RAM and iteration plugins.")
    print("  size:    Number of elements in the memory buffer")
    print("  index:   Number of bits in an index")
    print("  element: Number of bits in an element")
    print("  output:  The basename for created files.")

  size = int(sys.argv[1])
  index_bits = int(sys.argv[2])
  element_bits = int(sys.argv[3])
  output = str(sys.argv[4])

  REL = open(output + ".rel", "w")
  INS = open(output + ".ins", "w")
  WIT = open(output + ".wit", "w")

  makeMemChkIR0(size, index_bits, element_bits, REL, INS, WIT)
