#! /usr/bin/python3

# Copyright (C) 2022 Stealth Software Technologies, Inc.

import sys
import random
import numpy as np
import TestCaseGen as tcg

def twoMuxHelper(G, current_place, place_of_sel_bit, input_place, out, items):
  new_range = [current_place, current_place + 2 * items]
  G.new(new_range[0], new_range[1])
  G.addc(current_place, place_of_sel_bit, 1)
  place_of_sel_bit_plus_one = current_place
  current_place += 1
  for i in range(0, items):
    G.mul(current_place + i, input_place + i, place_of_sel_bit_plus_one)
    G.mul(current_place  + items + i, input_place + items + i, place_of_sel_bit)
    G.add(out + i, current_place + i, current_place  + items + i)

  current_place += 2 * items

  G.delete(new_range[0], new_range[1])
  return current_place

def oneMuxHelper(G, current_place, place_of_sel_bit, input_place, out, items):
  new_range = [current_place, current_place]
  G.new(new_range[0], new_range[1])
  G.addc(current_place, place_of_sel_bit, 1)
  place_of_sel_bit_plus_one = current_place
  current_place += 1
  for i in range(0, items):
    G.mul(out + i, input_place + i, place_of_sel_bit_plus_one)

  G.delete(new_range[0], new_range[1])
  return current_place

def treeMuxHelper(G, current_place, place_of_sel_bits, num_bits, input_place, out, cases, items):
  if(num_bits == 1):
    if(cases >= 2):
      current_place = twoMuxHelper(G, current_place, place_of_sel_bits, input_place, out, items)
      return current_place
    elif (cases == 1):
      current_place = oneMuxHelper(G, current_place, place_of_sel_bits, input_place, out, items)
      return current_place
    else:
      raise("Wrong num cases exception")
  else:
    new_range = [current_place, current_place + 2 * items - 1]
    G.new(new_range[0], new_range[1])

    new_place = current_place + 2 * items

    if(cases <= 2**(num_bits - 1)):
      new_place = treeMuxHelper(G, new_place, place_of_sel_bits + 1, num_bits - 1, input_place, current_place, cases, items)
      for i in range(items):
        G.assign(current_place + items + i, 0)
    else:
      new_place = treeMuxHelper(G, new_place, place_of_sel_bits + 1, num_bits - 1, input_place, current_place, 2**(num_bits - 1), items)
      new_place = treeMuxHelper(G, new_place, place_of_sel_bits + 1, num_bits - 1, input_place + (2**(num_bits - 1)) * items, current_place + items, cases - 2**(num_bits - 1), items)
    new_place = twoMuxHelper(G, new_place, place_of_sel_bits, current_place, out, items)
    G.delete(new_range[0], new_range[1])
  return new_place

def multiplexer(F, num_bits, cases, items):
  name = "mux_" + str(cases) + "_" + str(items)
  ins = [[0, num_bits]]
  for i in range(0, cases):
    ins.append([0, items])

  F.beginFunction(name, [[0, items]], ins)
  treeMuxHelper(F, (cases + 1) * items + num_bits, items, num_bits, items + num_bits, 0, cases, items)

  F.end()
  return name

def multiplexerPlugin(F, num_bits, cases, items):
  name = "mux_" + str(cases) + "_" + str(items)
  ins = [[0, num_bits]]
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

def write_bool(val, num_bits, filename):
  out = []
  for i in range(0, num_bits):
    out.append(val % 2)
    val = val // 2

  for i in range(0, num_bits):
    filename.write("<" + str(out[num_bits - 1 - i]) + ">;\n")


def generateInsWits(num_bits, cases, items, f_ins, f_wit):
  f_ins.write("@begin\n")
  f_ins.write("@end\n")

  f_wit.write("@begin\n")
  selector = random.randint(0, cases) % cases
  write_bool(selector, num_bits, f_wit)

  correct_output = []
  max_item_val = 2**items
  for i in range(0, cases):
    val = random.randint(0, max_item_val) % max_item_val
    f_wit.write("\n")
    write_bool(val, items, f_wit)

    if i == selector and i < 2**num_bits:
      correct_output.append(val)
  f_wit.write("\n")

  for val in correct_output:
    write_bool(val, items, f_wit)
  if len(correct_output) == 0:
    write_bool(0, items, f_wit)
  f_wit.write("@end\n")

# Generate a multiplexer statement
def generateMultiplexerIR0Relation(ir, rel, num_bits, cases, items):
  if ir == "no_plugins":
    header(rel, 2, "circuit", "")
  else:
    header(rel, 2, "circuit", "mux_v0")
  rel.write("@begin\n")

  F = tcg.GateWriter(rel, 0)

  mux_name = ""
  if ir == "no_plugins":
    mux_name = \
        multiplexer(F, num_bits, cases, items)
  else:
    mux_name = multiplexerPlugin(F, num_bits, cases, items)

  # Read in the first selector bit (Not sure if this is necessary, imitating)
  #F.private(0)

  # Read in all the cases/items
  F.new(0, cases * items + num_bits - 1)
  for i in range(0, cases * items + num_bits):
    F.private(i)

  in_list = [[0, num_bits - 1]]
  for i in range(0, cases):
    in_list.append([num_bits + i * items, num_bits + i * items + items - 1])

  out_list = [[num_bits + cases * items, num_bits + cases * items  + items - 1]]

  F.call(mux_name, out_list, in_list);
  F.delete(0, cases * items + num_bits - 1)
  F.new(num_bits + (cases + 1) * items, num_bits + (cases + 2) * items - 1)

  for i in range(num_bits + (cases + 1) * items, num_bits + (cases + 2) * items):
    F.private(i)

  # write out the assert equals function
  F.newLine()
  F.beginFunction("assertEqBool", [], [(0, 1), (0, 1)])
  F.add(2, 0, 1)
  F.assertZero(2)
  F.end()

  for i in range(num_bits + cases * items, num_bits + (cases + 1) * items):
    F.call("assertEqBool", [], [[i], [i + items]])

  F.end()

def generateBooleanMultiplexerIR0(ir, num_bits, case_count, item_count, \
    rel, f_ins, f_wit):
  header(f_ins, 2, "public_input", "")
  header(f_wit, 2, "private_input", "")

  generateInsWits( \
      num_bits, case_count, item_count, f_ins, f_wit)
  generateMultiplexerIR0Relation(ir, rel, num_bits, case_count, item_count)

  rel.flush()
  rel.close()

  f_ins.flush()
  f_ins.close()

  f_wit.flush()
  f_wit.close()

if __name__ == "__main__":
  if len(sys.argv) != 6:
    print("Usage: multiplexer <ir>  <cases> <items> <output>\n")
    print("A multiplexer circuit with the same field for both case selector and items/candidates.")
    print("  ir:      Either \"no_plugins\" or \"mux_v0\"")
    print("  num_bits:       number of bits used for the selector.")
    print("  cases:   The number of cases (no restriction - inputs past 2^num_bits are ignored).")
    print("  items:   The number of items in each case.")
    print("  output:  The base name for files to be created.")

  if sys.argv[1] != "no_plugins" and sys.argv[1] != "mux_v0":
    print("unrecognized IR variant")
    sys.exit(1)

  num_bits = int(sys.argv[2])
  case_count = int(sys.argv[3])
  item_count = int(sys.argv[4])
  base_name = str(sys.argv[5])

  generateBooleanMultiplexerIR0(sys.argv[1], num_bits, case_count, item_count, \
      open(base_name + ".rel", "w"), \
      open(base_name + ".F.ins", "w"), open(base_name + ".F.wit", "w"))
