#! /usr/bin/python3

# Copyright (C) 2023, Stealth Software Technologies, Inc.

# This script will generate an IR statement for checking that every cell
# in an IR RAM buffer works as intended. It does so by writing various
# values to the buffer, and checking their correct rememberance.

import sys
import random
import TestCaseGen as tcg

def streamHeader(f, res, p):
  f.write("version 2.1.0;\n")
  f.write(res + ";\n")
  f.write("@type field " + str(p) + ";\n")
  f.write("@begin\n")

def relationHeader(f, res, p, l, plugin_v1):
  f.write("version 2.1.0;\n")
  f.write(res + ";\n")
  if plugin_v1:
    f.write("@plugin ram_arith_v1;\n")
  else:
    f.write("@plugin ram_arith_v0;\n")
  f.write("@plugin iter_v0;\n")
  f.write("@type field " + str(p) + ";\n")
  if plugin_v1:
    f.write("@type @plugin(ram_arith_v1, ram, 0);\n")
  else:
    f.write("@type @plugin(ram_arith_v0, ram, 0, 1, " + str(l) + ", " + str(l) + ");\n")
  f.write("@begin\n")

def makeMemchk(r, size, prime, plugin_v1):
  r.beginFunction("ram_init", [[1, 1]], [[0, 1]])
  if plugin_v1:
    r.pluginBinding("ram_arith_v1", "init", [size])
  else:
    r.pluginBinding("ram_arith_v0", "init", [size])
  r.newLine()

  r.beginFunction("ram_read", [[0, 1]], [[1, 1], [0, 1]])
  if plugin_v1:
    r.pluginBinding("ram_arith_v1", "read", [])
  else:
    r.pluginBinding("ram_arith_v0", "read", [])
  r.newLine()

  r.beginFunction("ram_write", [], [[1, 1], [0, 1], [0, 1]])
  if plugin_v1:
    r.pluginBinding("ram_arith_v1", "write", [])
  else:
    r.pluginBinding("ram_arith_v0", "write", [])
  r.newLine()

  r.assign(0, 1)
  r.call("ram_init", [[0]], [[0]])
  r.newLine()

  r.beginFunction("expect_1", [], [[1, 1], [0, 1]])
  r.call("ram_read", [[1]], [[0], [0]])
  r.addc(2, 1, prime - 1)
  r.assertZero(2)
  r.end()

  r.beginFunction("iter_expect_1", [], [[1, 1]])
  r.pluginBinding("iter_v0", "map_enumerated", ["expect_1", 1, size])
  r.newLine()

  r.call("iter_expect_1", [], [[0]])

  r.beginFunction("write_0", [], [[1, 1], [0, 1]])
  r.assign(1, 0)
  r.call("ram_write", [], [[0], [0], [1]])
  r.end()

  r.beginFunction("iter_write_0", [], [[1, 1]])
  r.pluginBinding("iter_v0", "map_enumerated", ["write_0", 1, size])
  r.newLine()

  r.call("iter_write_0", [], [[0]])

  r.beginFunction("expect_0", [], [[1, 1], [0, 1]])
  r.call("ram_read", [[1]], [[0], [0]])
  r.assertZero(1)
  r.end()

  r.beginFunction("iter_expect_0", [], [[1, 1]])
  r.pluginBinding("iter_v0", "map_enumerated", ["expect_0", 1, size])
  r.newLine()

  r.call("iter_expect_0", [], [[0]])

  r.beginFunction("write_idx", [], [[1, 1], [0, 1]])
  r.call("ram_write", [], [[0], [0], [0]])
  r.end()

  r.beginFunction("iter_write_idx", [], [[1, 1]])
  r.pluginBinding("iter_v0", "map_enumerated", ["write_idx", 1, size])

  r.call("iter_write_idx", [], [[0]])

  r.beginFunction("expect_idx", [], [[1, 1], [0, 1]])
  r.mulc(1, 0, prime - 1)
  r.call("ram_read", [[2]], [[0], [0]])
  r.add(3, 1, 2)
  r.assertZero(3)
  r.end()

  r.beginFunction("iter_expect_idx", [], [[1, 1]])
  r.pluginBinding("iter_v0", "map_enumerated", ["expect_idx", 1, size])

  r.call("iter_expect_idx", [], [[0]])

  r.beginFunction("write_val", [], [[1, 1], [0, 1], [0, 1]])
  r.call("ram_write", [], [[0], [1], [0]])
  r.end()

  r.beginFunction("iter_write_val", [], [[1, 1], [0, 1]])
  r.pluginBinding("iter_v0", "map_enumerated", ["write_val", 2, size])

  r.beginFunction("expect_val", [], [[1, 1], [0, 1], [0, 1]])
  r.mulc(2, 0, prime - 1)
  r.call("ram_read", [[3]], [[0], [1]])
  r.add(4, 2, 3)
  r.assertZero(4)
  r.end()

  r.beginFunction("iter_expect_val", [], [[1, 1], [0, 1]])
  r.pluginBinding("iter_v0", "map_enumerated", ["expect_val", 2, size])

  r.assign(1, 0)
  r.call("iter_write_val", [], [[0], [1]])
  r.call("iter_expect_val", [], [[0], [1]])
  r.assign(2, 1)
  r.call("iter_write_val", [], [[0], [2]])
  r.call("iter_expect_val", [], [[0], [2]])
  if prime > 2:
    r.assign(3, 2)
    r.call("iter_write_val", [], [[0], [3]])
    r.call("iter_expect_val", [], [[0], [3]])

  val = 1
  if_cond = False
  while val < prime:
    val = val << 1
    if if_cond:
      val = val + 1
    if_cond = not if_cond
  val = val >> 1

  r.assign(4, val)
  r.call("iter_write_val", [], [[0], [4]])
  r.call("iter_expect_val", [], [[0], [4]])

  val = (~val) % prime

  r.assign(5, val)
  r.call("iter_write_val", [], [[0], [5]])
  r.call("iter_expect_val", [], [[0], [5]])

  r.assign(6, prime - 1)
  r.call("iter_write_val", [], [[0], [6]])
  r.call("iter_expect_val", [], [[0], [6]])

  r.beginFunction("write_witness", [], [[1, 1]])
  r.private(0)
  r.private(1)
  r.call("ram_write", [], [[0], [0], [1]])
  r.end()

  r.beginFunction("iter_write_witness", [], [[1, 1]])
  r.pluginBinding("iter_v0", "map", ["write_witness", 1, size])

  r.beginFunction("expect_witness", [], [[1, 1]])
  r.private(0)
  r.private(1)
  r.mulc(2, 1, prime - 1)
  r.call("ram_read", [[3]], [[0], [0]])
  r.add(4, 2, 3)
  r.assertZero(4)
  r.end()

  r.beginFunction("iter_expect_witness", [], [[1, 1]])
  r.pluginBinding("iter_v0", "map", ["expect_witness", 1, size])

  r.call("iter_write_witness", [], [[0]])
  r.call("iter_expect_witness", [], [[0]])
  r.call("iter_expect_witness", [], [[0]])
  r.call("iter_expect_witness", [], [[0]])

  r.beginFunction("iter_ram_read", [[0, size]], [[1, 1]])
  r.pluginBinding("iter_v0", "map_enumerated", ["ram_read", 1, size])

  r.call("iter_ram_read", [[7, 6 + size]], [[0]])

  r.beginFunction("expect_witness_noram", [], [[0, 1]])
  r.mulc(1, 0, prime - 1)
  r.private(2)
  r.add(3, 1, 2)
  r.assertZero(3)
  r.end()

  r.beginFunction("iter_expect_witness_noram", [], [[0, size]])
  r.pluginBinding("iter_v0", "map", ["expect_witness_noram", 0, size])

  r.call("iter_expect_witness_noram", [], [[7, 6 + size]])

  r.end()

def makeWitness(f, size, prime):
  bfr = [prime - 1] * size
  acs_ptrn = []
  for i in range(0, size):
    idx = random.randrange(0, size)
    val = random.randrange(0, prime)
    f.write("<" + str(idx) + ">;\n")
    f.write("<" + str(val) + ">;\n")
    bfr[idx] = val
    acs_ptrn.append(idx)
  for i in range(0, size):
    f.write("<" + str(acs_ptrn[i]) + ">;\n")
    f.write("<" + str(bfr[acs_ptrn[i]]) + ">;\n")
  for i in range(0, size):
    idx = random.randrange(0, size)
    f.write("<" + str(idx) + ">;\n")
    f.write("<" + str(bfr[idx]) + ">;\n")
  for i in range(0, size):
    f.write("<" + str(i) + ">;\n")
    f.write("<" + str(bfr[i]) + ">;\n")
  for i in range(0, size):
    f.write("<" + str(bfr[i]) + ">;\n")
  f.write("@end\n")

def makeMemChkIR0(size, prime, rel_file, ins_file, wit_file, plugin_v1):
  relationHeader(rel_file, "circuit", prime, size, plugin_v1)
  streamHeader(ins_file, "public_input", prime)
  streamHeader(wit_file, "private_input", prime)
  ins_file.write("@end\n")
  makeWitness(wit_file, size, prime)
  makeMemchk(tcg.GateWriter(rel_file, 0), size, prime, plugin_v1)

  rel_file.flush()
  rel_file.close()

  ins_file.flush()
  ins_file.close()

  wit_file.flush()
  wit_file.close()

if __name__ == "__main__":
  if len(sys.argv) != 4 and len(sys.argv) != 5:
    print("Usage: memchk <size> <prime> <output> [v1]\n")
    print("Generate a memory test circuit using the RAM and iteration plugins.")
    print("  size:    Number of elements in the memory buffer")
    print("  prime:   The prime field.")
    print("  output:  The basename for created files.")
    sys.exit(1)

  size = int(sys.argv[1])
  prime = int(sys.argv[2])
  output = str(sys.argv[3])
  plugin_v1 = False
  if len(sys.argv) == 5:
    plugin_v1 = True

  REL = open(output + ".rel", "w")
  INS = open(output + ".ins", "w")
  WIT = open(output + ".wit", "w")

  makeMemChkIR0(size, prime, REL, INS, WIT, plugin_v1)
