#! /usr/bin/python3

# Copyright (C) 2023, Stealth Software Technologies, Inc.

# This script will generate an IR statement for testing that the less than,
# less than equal, and division extended arithmetic operations actually
# function correctly.

import sys
import random
import TestCaseGen as tcg

def streamHeader(f, res, p):
  f.write("version 2.1.0;\n")
  f.write(res + ";\n")
  f.write("@type field " + str(p) + ";\n")
  f.write("@begin\n")

def decomp(n):
  ret = []
  while n > 0:
    # decompose in little-endian
    ret.append(n & 1)
    n = n >> 1
  # then reverse for big
  ret.reverse()
  return ret

def decomp_pad(n, b):
  ret = decomp(n)
  # pad the front with an appropriate number of 0 bits
  return ([0] * (b - len(ret))) + ret

def relationHeader(f, p):
  f.write("version 2.1.0;\n")
  f.write("circuit;\n")
  f.write("@plugin extended_arithmetic_v1;\n")
  f.write("@type field " + str(p) + ";\n")
  f.write("@begin\n")
  f.write("@function(lt, @out: 0:1, @in: 0:1, 0:1)\n")
  f.write("  @plugin(extended_arithmetic_v1, less_than);\n")
  f.write("@function(lte, @out: 0:1, @in: 0:1, 0:1)\n")
  f.write("  @plugin(extended_arithmetic_v1, less_than_equal);\n")
  f.write("@function(div, @out: 0:1, 0:1, @in: 0:1, 0:1)\n")
  f.write("  @plugin(extended_arithmetic_v1, division);\n")
  f.write("@function(bit_decomp, @out: 0:" + str(len(decomp(p))))
  f.write(", @in: 0:1)\n  @plugin(extended_arithmetic_v1, bit_decompose);\n")
  f.write("@function(assert_equal, @in: 0:1, 0:1)\n")
  f.write("  $2<-@mulc(0:$1, <" + str(p - 1) + ">);\n")
  f.write("  $3<-@add(0:$0, $2);\n")
  f.write("  @assert_zero(0:$3);\n")
  f.write("@end\n\n");

def testBitDecomp(i, p, p_bits, next_id, ins, r):
  dcmp = decomp_pad(i, p_bits)

  ins.write("<" + str(i) + ">;\n")
  for b in dcmp:
    ins.write("<" + str(b) + ">;\n")

  r.new(next_id, next_id + 2 * p_bits)
  r.public(next_id)
  r.call("bit_decomp", [[next_id + 1, next_id + p_bits]], [[next_id]])
  dc_start = next_id + 1
  i_start = dc_start + p_bits
  for i in range(0, p_bits):
    r.public(i_start + i)
  for i in range(0, p_bits):
    r.call("assert_equal", [], [[dc_start + i], [i_start + i]])
  r.delete(next_id, next_id + 2 * p_bits)
  return next_id + 2 * p_bits + 1

def makeTest(i, j, next_id, ins, r):
  ins.write("<" + str(i) + ">;\n")
  ins.write("<" + str(j) + ">;\n")
  ins.write("<" + str(1 if i < j else 0) + ">;\n")
  ins.write("<" + str(1 if i <= j else 0) + ">;\n")

  if j != 0:
    ins.write("<" + str(i // j) + ">;\n")
    ins.write("<" + str(i % j) + ">;\n")
    r.new(next_id, next_id + 9)
  else:
    r.new(next_id, next_id + 5)

  r.public(next_id)
  r.public(next_id + 1)
  r.call("lt", [[next_id + 2]], [[next_id], [next_id + 1]])
  r.public(next_id + 3)
  r.call("assert_equal", [], [[next_id + 2], [next_id + 3]])
  r.call("lte", [[next_id + 4]], [[next_id], [next_id + 1]])
  r.public(next_id + 5)
  r.call("assert_equal", [], [[next_id + 4], [next_id + 5]])

  if j != 0:
    r.call("div", [[next_id + 6], [next_id + 7]], [[next_id], [next_id + 1]])
    r.public(next_id + 8)
    r.public(next_id + 9)
    r.call("assert_equal", [], [[next_id + 6], [next_id + 8]])
    r.call("assert_equal", [], [[next_id + 7], [next_id + 9]])
    r.delete(next_id, next_id + 9)
    r.newLine()
    return next_id + 10
  else:
    r.delete(next_id, next_id + 5)
    r.newLine()
    return next_id + 6

def makeLessThanDivTestIR0(p, rel, ins, wit):
  streamHeader(wit, "private_input", p)
  wit.write("@end\n")
  streamHeader(ins, "public_input", p)
  relationHeader(rel, p)

  r = tcg.GateWriter(rel, 0)
  next_id = 0

  for i in range(0, 20):
    for j in range(0, 20):
      next_id = makeTest(i, j, next_id, ins, r)

  for i in range(p - 1, p - 20, -1):
    for j in range(p - 1, p - 20, -1):
      next_id = makeTest(i, j, next_id, ins, r)

  for i in range(0, 20):
    for j in range(p - 1, p - 20, -1):
      next_id = makeTest(i, j, next_id, ins, r)

  for i in range(p - 1, p - 20, -1):
    for j in range(0, 20):
      next_id = makeTest(i, j, next_id, ins, r)

  for i in range(0, 20):
    for j in range(0, 20):
      a = random.randrange(0, p)
      b = random.randrange(1, p)
      next_id = makeTest(a, b, next_id, ins, r)

  prime_bits = len(decomp(p))
  for i in range (0, 20):
    next_id = testBitDecomp(i, p, prime_bits, next_id, ins, r)

  for i in range (p - 1, p - 20, -1):
    next_id = testBitDecomp(i, p, prime_bits, next_id, ins, r)

  for i in range (0, 20):
    a = random.randrange(0, p)
    next_id = testBitDecomp(a, p, prime_bits, next_id, ins, r)

  ins.write("@end\n")
  rel.write("@end\n")

  rel.flush();
  rel.close();

  ins.flush();
  ins.close();

  wit.flush();
  wit.close();

if __name__ == "__main__":
  if len(sys.argv) != 3:
    print("USAGE: less_than_div_test <prime> <output>\n")
    print("Generate a test circuit for comparisons and division plugins.")
    print("  prime:  the prime field.")
    print("  output: the basename for created files.")

  prime = int(sys.argv[1])
  output = str(sys.argv[2])

  REL = open(output + ".rel", "w")
  INS = open(output + ".ins", "w")
  WIT = open(output + ".wit", "w")

  makeLessThanDivTestIR0(prime, REL, INS, WIT)
