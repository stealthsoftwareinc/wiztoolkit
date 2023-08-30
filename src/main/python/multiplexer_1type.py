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

def multiplexer(F, cases, items, exp, f_prime):
  name = "mux_" + str(cases) + "_" + str(items)
  ins = [[0, 1]]
  for i in range(0, cases):
    ins.append([0, items])

  F.beginFunction(name, [[0, items]], ins)
  sel_start = (cases + 1) * items + 1
  F.new(1 + sel_start, sel_start + cases * 3)
  F.new(1 + sel_start + cases * 3, sel_start + cases * 4)
  for i in range(1, cases + 1):
    F.addc(sel_start + i, items, (1 + f_prime - i) % f_prime)
    F.call(exp, [[sel_start + i + cases]], [[sel_start + i]])
    F.mulc(sel_start + i + 2 * cases, sel_start + i + cases, f_prime - 1)
    F.addc(sel_start + i + 3 * cases, sel_start + i + 2 * cases, 1)
  F.delete(1 + sel_start, sel_start + cases * 3)

  place = sel_start + cases * 4 + 1
  for i in range(0, items):
    mux_list = []
    for j in range(0, cases):
      mux_list.append(1 + i + (j + 1) * items)
    place = muxHelper(F, place, 1 + sel_start + cases * 3, i, mux_list)

  F.end()
  return name

def multiplexerPlugin(F, cases, items):
  name = "mux_" + str(cases) + "_" + str(items)
  ins = [[0, 1]]
  for i in range(0, cases):
    ins.append([0, items])

  F.beginFunction(name, [[0, items]], ins)
  F.pluginBinding("mux_v0", "permissive", [])
  return name

def header(f, prime, resource, plugin):
  f.write("version 2.1.0;\n" + resource + ";\n")
  if plugin != "":
    f.write("@plugin " + plugin + ";\n")
  f.write("@type field " + str(prime) + ";\n")

def generateInsWits(f_prime, cases, items, f_ins, f_wit):
  f_ins.write("@begin\n")
  f_ins.write("@end\n")

  f_wit.write("@begin\n")
  selector = random.randint(0, cases) % cases
  f_wit.write("<" + str(selector) + ">;\n")

  correct_output = []
  for i in range(0, cases):
    for j in range(0, items):
      val = random.randint(0, f_prime) % f_prime
      f_wit.write("<" + str(val) + ">;\n")
      if i == selector:
        correct_output.append(val)
    f_wit.write("\n")

  for val in correct_output:
    f_wit.write("<" + str(val) + ">;\n")
  f_wit.write("@end\n")

# Generate a multiplexer statement
def generateMultiplexerIR0Relation(ir, rel, f_prime, cases, items):
  if ir == "no_plugins":
    header(rel, f_prime, "circuit", "")
  else:
    header(rel, f_prime, "circuit", "mux_v0")
  rel.write("@begin\n")

  F = tcg.GateWriter(rel, 0)

  mux_name = ""
  if ir == "no_plugins":
    mux_name = \
        multiplexer(F, cases, items, exponentiate(F, f_prime - 1), f_prime)
  else:
    mux_name = multiplexerPlugin(F, cases, items)

  # Read in the selector
  F.private(0)

  # Read in all the cases/items
  F.new(1, cases * items)
  for i in range(1, cases * items + 1):
    F.private(i)

  in_list = [[0]]
  for i in range(0, cases):
    in_list.append([1 + i * items, i * items + items])

  out_list = [[1 + cases * items, cases * items  + items]]

  F.call(mux_name, out_list, in_list);
  F.delete(1, cases * items)
  F.new(1 + (cases + 1) * items, (cases + 2) * items)

  for i in range(1 + (cases + 1) * items, (cases + 2) * items + 1):
    F.private(i)

  # write out the assert equals function
  F.newLine()
  F.beginFunction("assertEq", [], [(0, 1), (0, 1)])
  F.mulc(2, 1, f_prime - 1)
  F.add(3, 0, 2)
  F.assertZero(3)
  F.end()

  for i in range(1 + cases * items, (cases + 1) * items + 1):
    F.call("assertEq", [], [[i], [i + items]])

  F.end()

def generateMultiplexerIR0(ir, f_prime, case_count, item_count, \
    rel, f_ins, f_wit):
  header(f_ins, f_prime, "public_input", "")
  header(f_wit, f_prime, "private_input", "")

  generateInsWits( \
      f_prime, case_count, item_count, f_ins, f_wit)
  generateMultiplexerIR0Relation(ir, rel, f_prime, case_count, item_count)

  rel.flush()
  rel.close()

  f_ins.flush()
  f_ins.close()

  f_wit.flush()
  f_wit.close()

if __name__ == "__main__":
  if len(sys.argv) != 6:
    print("Usage: multiplexer <ir> <F> <cases> <items> <output>\n")
    print("A multiplexer circuit with the same field for both case selector and items/candidates.")
    print("  ir:      Either \"no_plugins\" or \"mux_v0\"")
    print("  F:       Field used in this example.")
    print("  cases:   The number of cases (must be less then F and G).")
    print("  items:   The number of items in each case.")
    print("  output:  The base name for files to be created.")

  if sys.argv[1] != "no_plugins" and sys.argv[1] != "mux_v0":
    print("unrecognized IR variant")
    sys.exit(1)

  F_prime = int(sys.argv[2])
  case_count = int(sys.argv[3])
  item_count = int(sys.argv[4])
  base_name = str(sys.argv[5])

  generateMultiplexerIR0(sys.argv[1], F_prime, case_count, item_count, \
      open(base_name + ".rel", "w"), \
      open(base_name + ".F.ins", "w"), open(base_name + ".F.wit", "w"))
