#! /usr/bin/python3

# Copyright (C) 2022 Stealth Software Technologies, Inc.

import sys
import random
import numpy as np
import TestCaseGen as tcg

def exponentiate(gen, exp):
  name = "exp_" + str(exp)
  gen.beginFunction(name, [[0, 1]], [[0, 1]])
  base = 1
  gen.assign(2, 1)
  aux = 2
  place = 3
  while exp > 1:
    gen.comment("exp: " + str(exp))
    if exp % 2 == 0:
      gen.mul(place, base, base)
      base = place
      place += 1
      exp = exp // 2
    else:
      gen.mul(place, base, aux)
      gen.mul(place + 1, base, base)
      aux = place
      base = place + 1
      place += 2

      exp = (exp - 1) // 2

  gen.mul(0, base, aux)
  gen.end()

  return name

def muxHelper(G, place, sel_start, out, items):
  new_range = [place, place + len(items) * 2 - 3]
  G.new(new_range[0], new_range[1])
  G.mul(place, sel_start, items[0])
  G.mul(place + 1, sel_start + 1, items[1])
  place += 2

  for i in range(2, len(items)):
    G.add(place, place - 2, place - 1)
    G.mul(place + 1, sel_start + i, items[i])
    place += 2

  G.add(out, place - 2, place - 1)
  G.delete(new_range[0], new_range[1])
  return place

def multiplexer(F, G, cases, items, exp, f_prime):
  name = "mux_" + str(cases) + "_" + str(items)
  ins = [[0, 1]]
  for i in range(0, cases):
    ins.append([1, items])

  F.beginFunction(name, [[1, items]], ins)
  F.new(1, cases * 4)
  sel_start = (cases + 1) * items
  G.new(sel_start, sel_start + cases - 1)
  for i in range(1, cases + 1):
    F.addc(i, 0, (1 + f_prime - i) % f_prime)
    F.call(exp, [[i + cases]], [[i]])
    F.mulc(i + 2 * cases, i + cases, f_prime - 1)
    F.addc(i + 3 * cases, i + 2 * cases, 1)
    tcg.convert2(G, i - 1 + (cases + 1) * items, F, i + 3 * cases)
  F.delete(1, cases * 4)

  place = cases + items * (cases + 1)
  for i in range(0, items):
    mux_list = []
    for j in range(0, cases):
      mux_list.append(i + (j + 1) * items)
    place = muxHelper(G, place, sel_start, i, mux_list)

  F.end()
  return name

def header(f, prime, resource):
  f.write("version 2.1.0;\n" + resource + ";\n")
  f.write("@type field " + str(prime) + ";\n")

def generateInsWits(f_prime, g_prime, cases, items, f_ins, f_wit, g_ins, g_wit):
  f_ins.write("@begin\n")
  f_ins.write("@end\n")

  g_ins.write("@begin\n")
  g_ins.write("@end\n")

  f_wit.write("@begin\n")
  selector = random.randint(0, cases) % cases
  f_wit.write("<" + str(selector) + ">;\n")
  f_wit.write("@end\n")

  g_wit.write("@begin\n")

  correct_output = []
  for i in range(0, cases):
    for j in range(0, items):
      val = random.randint(0, g_prime) % g_prime
      g_wit.write("<" + str(val) + ">;\n")
      if i == selector:
        correct_output.append(val)
    g_wit.write("\n")

  for val in correct_output:
    g_wit.write("<" + str(val) + ">;\n")
  g_wit.write("@end\n")

# Generate a multiplexer statement
def generateMultiplexerIR0Relation(rel, f_prime, g_prime, cases, items):
  header(rel, f_prime, "circuit")
  rel.write("@type field " + str(g_prime) + ";\n")
  rel.write("@convert(@out: 1:1, @in: 0:1);\n")
  rel.write("@begin\n")

  F = tcg.GateWriter(rel, 0)
  G = tcg.GateWriter(rel, 1)

  mux_name = multiplexer( \
      F, G, cases, items, exponentiate(F, f_prime - 1), f_prime)

  # Read in the selector
  F.private(0)

  # Read in all the cases/items
  G.new(0, cases * items - 1)
  for i in range(0, cases * items):
    G.private(i)

  in_list = [[0]]
  for i in range(0, cases):
    in_list.append([i * items, i * items + items - 1])

  out_list = [[cases * items, cases * items  + items - 1]]

  G.call(mux_name, out_list, in_list);
  G.delete(0, cases * items - 1)
  G.new((cases + 1) * items, (cases + 2) * items - 1)

  for i in range((cases + 1) * items, (cases + 2) * items):
    G.private(i)

  # write out the assert equals function
  G.newLine()
  G.beginFunction("assertEq", [], [(1, 1), (1, 1)])
  G.mulc(2, 1, g_prime - 1)
  G.add(3, 0, 2)
  G.assertZero(3)
  G.end()

  for i in range(cases * items, (cases + 1) * items):
    G.call("assertEq", [], [[i], [i + items]])

  F.end()

def generateMultiplexerIR0(f_prime, g_prime, case_count, item_count, \
    rel, f_ins, f_wit, g_ins, g_wit):
  header(f_ins, f_prime, "public_input")
  header(f_wit, f_prime, "private_input")
  header(g_ins, g_prime, "public_input")
  header(g_wit, g_prime, "private_input")

  generateInsWits( \
      f_prime, g_prime, case_count, item_count, f_ins, f_wit, g_ins, g_wit)
  generateMultiplexerIR0Relation(rel, f_prime, g_prime, case_count, item_count)

if __name__ == "__main__":
  if len(sys.argv) != 7:
    print("Usage: multiplexer <ir> <F> <G> <cases> <items> <output>\n")
    print("A multiplexer circuit with a field for the case selector and a different field for the input/output items\n")
    print("  ir:      Either \"no_plugins\" or coming soon plugin implementation")
    print("  F:       Field used for the case selector.")
    print("  G:       Field used for the input/output items.")
    print("  cases:   The number of cases (must be less then F and G).")
    print("  items:   The number of items in each case.")
    print("  output:  The base name for files to be created.")

  if sys.argv[1] != "no_plugins":
    print("unrecognized IR variant")
    sys.exit(1)

  F_prime = int(sys.argv[2])
  G_prime = int(sys.argv[3])
  case_count = int(sys.argv[4])
  item_count = int(sys.argv[5])
  base_name = str(sys.argv[6])

  generateMultiplexerIR0(F_prime, G_prime, case_count, item_count, \
      open(base_name + ".rel", "w"), \
      open(base_name + ".F.ins", "w"), open(base_name + ".F.wit", "w"), \
      open(base_name + ".G.ins", "w"), open(base_name + ".G.wit", "w"))
