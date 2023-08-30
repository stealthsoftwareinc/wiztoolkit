#! /usr/bin/python3

# Copyright (C) 2021-2022 Stealth Software Technologies, Inc.

import sys
import random
import numpy as np
import TestCaseGen as tcg

# Write the header for an IR resource
def header(f, resource, prime):
  f.write("version 2.1.0;\n" + resource + ";\n")
  f.write("@type field " + str(prime) + ";\n@begin\n")

# Generate the instance and witness 
def generateInsWit(ins_file, wit_file, m_dim, n_dim, c_dim, prime):
  M_mat = np.ndarray(m_dim, dtype=object)
  for i in range(0, m_dim[0]):
    for j in range(0, m_dim[1]):
      M_mat[i][j] = random.randint(0, prime) % prime
  N_mat = np.ndarray(n_dim, dtype=object)
  for i in range(0, n_dim[0]):
    for j in range(0, n_dim[1]):
      N_mat[i][j] = random.randint(0, prime) % prime

  def mod(x):
    return x % prime

  C_mat = mod(np.matmul(M_mat, N_mat, dtype=object))

  for i in range(0, m_dim[0]):
    for j in range(0, m_dim[1]):
      ins_file.write("< " + str(M_mat[i][j]) + " >;\n")

  for i in range(0, c_dim[0]):
    for j in range(0, c_dim[1]):
      ins_file.write("< " + str(C_mat[i][j]) + " >;\n")

  for i in range(0, n_dim[0]):
    for j in range(0, n_dim[1]):
      wit_file.write("< " + str(N_mat[i][j]) + " >;\n")

  ins_file.write("@end\n")
  wit_file.write("@end\n")

# helper function to index into a matrix.
def mat_idx(ptr, idx, sz):
  return ptr + (idx[0] * sz[1]) + idx[1]

# returns the size of a matrix, given its dimensions
def mat_dim(sz):
  return sz[0] * sz[1]

# returns the size of a matrix required to carry a single column
def mat_col_dim(sz):
  return 1 + (sz[0] - 1) * sz[1]

# generate a dot-product for the "textbook" algorithm
def generateDotProdTB(out, b, c):
  out.beginFunction("dotProd", [(0, 1)], [(0, b), (0, mat_col_dim([b, c]))])

  place = 0
  out.comment("output wire $" + str(place))
  place += 1

  out.comment("M row (" + str(b) + "): $" + str(place) + " ... $" \
      + str(place + b - 1))
  M_ROW = place
  place += b

  out.comment("N col (" + str(b) + "x" + str(c) + "): $"
      + str(place) + " ... $" + str(place + mat_col_dim([b, c]) - 1))
  N_COL = place
  place += mat_col_dim([b, c])

  out.new(place, place + 2 * b - 3)

  for i in range(0, b):
    out.mul(place, M_ROW + i, mat_idx(N_COL, [i, 0], [b, c]))
    place += 1
    if i != 0 and i != b - 1:
      out.add(place, place - 2, place - 1)
      place += 1
  out.add(0, place - 2, place - 1)
  out.end()

# generate a dot-product for the "partial transpose" algorithm.
def generateDotProdPT(out, b):
  out.beginFunction("dotProd", [(0, 1)], [(0, b), (0, b)])

  place = 0
  out.comment("output wire: $" + str(place))
  place += 1

  out.comment("M row(" + str(b) + "): $" + str(place) + " ... $"
      + str(place + b - 1))
  M_ROW = place
  place += b

  out.comment("N col(" + str(b) + "): $" + str(place) + " ... $"
      + str(place + b - 1))
  N_COL = place
  place += b

  out.new(place, place + 2 * b - 3)

  for i in range(0, b):
    out.mul(place, M_ROW + i, N_COL + i)
    place += 1
    if i != 0 and i != b - 1:
      out.add(place, place - 2, place - 1)
      place += 1

  out.add(0, place - 2, place - 1)
  out.end()

# generate a dot-product for the "partial transpose" algorithm.
def generateDotProdPlugin(out, b):
  out.beginFunction("dotProd", [(0, 1)], [(0, b), (0, b)])
  out.pluginBinding("wizkit_vectors", "dotproduct", [])

#function to generate partial transpose
def generatePartialTranspose(out, b, c):
  out.beginFunction("partTpose", [(0, b)], [(0, mat_col_dim([b, c]))])

  place = 0
  out.comment("Output wires (" + str(b) + "): $" + str(place) + " ... $"
      + str(place + b - 1))
  OUT = place
  place += b

  out.comment("N col (" + str(b) + "x" + str(c) + "): $"
      + str(place) + " ... $" + str(place + mat_col_dim([b, c]) - 1))
  N_COL = place
  place += mat_col_dim([b, c])

  for i in range(0, b):
    out.copy(OUT + i, mat_idx(N_COL, [i, 0], [b, c]))
  out.end()

# generate the row helper
def generateRowTB(out, b, c):
  out.beginFunction("row", [[0, c]], [[0, b], [0, mat_dim([b, c])]])
  out.comment("$0 ... $" + str(c - 1) + ": output")
  out.comment("$" + str(c) + " ... $" + str(c + b - 1) + ": M row")
  out.comment("$" + str(c + b) + " ... $"
      + str(c + b  - 1 + b * c) + ": N matrix")
  for j in range(0, c):
    out.comment("j: " + str(j))
    out.call("dotProd", [[j]],
        [(c, c + b - 1),
          (mat_idx(c + b, [0, j], [b, c]),
           mat_idx(c + b, [b - 1, j], [b, c]))])
  out.end()

# Generate the matrix-mul body for flat/text book
def generateBodyFlatTB(out, c_start, m_start, n_start, place, a, b, c):
  for i in range(0, a):
    for j in range(0, c):
      out.comment("i: " + str(i) + ", j: " + str(j))
      rsrv = [place, place + 2 * b - 3]
      out.new(rsrv[0], rsrv[1])
      for k in range(0, b):
        out.mul(place, \
            mat_idx(m_start, [i, k], [a, b]), mat_idx(n_start, [k, j], [b, c]))
        place += 1
        if k != 0 and k != b - 1:
          out.add(place, place - 2, place - 1)
          place += 1
      out.add(mat_idx(c_start, [i, j], [a, c]), place - 2, place - 1)

      out.delete(rsrv[0], rsrv[1])

# Generate the matrix-mul body for flat/partial transpose
def generateBodyFlatPT(out, c_start, m_start, n_start, place, a, b, c):
  for j in range(0, c):
    out.comment("j: " + str(j))
    Ncol = place
    place += b
    out.new(Ncol, place - 1)
    for k in range(0, b):
      out.copy(Ncol + k, mat_idx(n_start, [k, j], [b, c]))
    for i in range(0, a):
      Mrow = mat_idx(m_start, [i, 0], [a, b])

      out.comment("i: " + str(i) + ", j: " + str(j))
      rsrv = [place, place + 2 * b - 3]
      out.new(rsrv[0], rsrv[1])

      for k in range(0, b):
        out.mul(place, Mrow + k, Ncol + k)
        place += 1
        if k != 0 and k != b - 1:
          out.add(place, place - 2, place - 1)
          place += 1
      out.add(mat_idx(c_start, [i, j], [a, c]), place - 2, place - 1)
      out.delete(rsrv[0], rsrv[1])
    out.delete(Ncol, Ncol + b - 1)

# Generate the matrix-mul body for dotProd/text book
def generateBodyDotProdTB(out, c_start, m_start, n_start, place, a, b, c):
  for i in range(0, a):
    for j in range(0, c):
      out.comment("i: " + str(i) + ", j: " + str(j))
      out.call("dotProd", [[mat_idx(c_start, [i, j], [a, c])]], \
          [(mat_idx(m_start, [i, 0], [a, b]), \
              mat_idx(m_start, [i, b - 1], [a, b])), \
           (mat_idx(n_start, [0, j], [b, c]), \
              mat_idx(n_start, [b - 1, j], [b, c]))])

# Generate the matrix-mul body for dotProd/partial transpose
def generateBodyDotProdPT(out, c_start, m_start, n_start, place, a, b, c):
  for j in range(0, c):
    out.comment("j: " + str(j))
    Ncol = place
    place += b
    out.call("partTpose", [(Ncol, place - 1)],
        [(mat_idx(n_start, [0, j], [b, c]),
            mat_idx(n_start, [b - 1, j], [b, c]))])

    for i in range(0, a):
      Mrow = mat_idx(m_start, [i, 0], [a, b])

      out.comment("i: " + str(i) + ", j: " + str(j))
      out.call("dotProd", [[mat_idx(c_start, [i, j], [a, c])]],
          [(Mrow, Mrow + b - 1), (Ncol, Ncol + b - 1)])

    out.delete(Ncol, Ncol + b - 1)

# Generate the matrix-mul body for dotProd/row helper textbook
def generateBodyRowTB(out, c_start, m_start, n_start, place, a, b, c):
  for i in range(0, a):
    out.comment("i: " + str(i))
    out.call("row",
        [(mat_idx(c_start, [i, 0], [a, c]),
          mat_idx(c_start, [i, c - 1], [a, c]))],
        [(mat_idx(m_start, [i, 0], [a, b]),
          mat_idx(m_start, [i, b - 1], [a, b])),
          (n_start, n_start - 1 + b * c)])

# generate the function for matrix-mul.
def generateMatrixMul(out, ir, a, b, c):
  out.beginFunction("matrixMul", [(0, mat_dim([a, c]))],
          [(0, mat_dim([a, b])), (0, mat_dim([b, c]))])

  place = 0

  C_START = place
  out.comment("C (" + str(a) + "x" + str(c) + "): $"
    + str(place) + " ... $" + str(place + mat_dim([a, c]) - 1))
  place += mat_dim([a, c])

  M_START = place
  out.comment("M (" + str(a) + "x" + str(b) + "): $"
    + str(place) + " ... $" + str(place + mat_dim([a, b]) - 1))
  place += mat_dim([a, b])

  N_START = place
  out.comment("N (" + str(b) + "x" + str(c) + "): $"
    + str(place) + " ... $" + str(place + mat_dim([b, c]) - 1))
  place += mat_dim([b, c])

  if ir == "flat_tb":
    generateBodyFlatTB(out, C_START, M_START, N_START, place, a, b, c)
  # Flat/partial transpose
  elif ir == "flat_pt":
    generateBodyFlatPT(out, C_START, M_START, N_START, place, a, b, c)
  # dot product/textbook
  elif ir == "dotprod_tb":
    generateBodyDotProdTB(out, C_START, M_START, N_START, place, a, b, c)
  # dot product row
  elif ir == "dotprod_row_tb":
    generateBodyRowTB(out, C_START, M_START, N_START, place, a, b, c)
  #dot product/partial transpose
  elif ir == "dotprod_pt" or ir == "plugin_pt":
    generateBodyDotProdPT(out, C_START, M_START, N_START, place, a, b, c)
  out.end()

# "top level" code for reading the instance/witness and checking results
def generateMainBody(out, a, b, c, p):
  place = 0

  M_START = place
  M_END = M_START + mat_dim([a, b]) - 1
  out.newLine()
  out.comment("M (" + str(a) + "x" + str(b) + "): $"
      + str(M_START) + " ... $" + str(M_END))

  out.new(M_START, M_END, True)
  for i in range(0, mat_dim([a, b])):
    out.public(place)
    place += 1

  N_START = place
  N_END = N_START + mat_dim([b, c]) - 1
  out.newLine()
  out.comment("N (" + str(b) + "x" + str(c) + "): $"
      + str(N_START) + " ... $" + str(N_END))

  out.new(N_START, N_END, True)
  for i in range(0, mat_dim([b, c])):
    out.private(place)
    place += 1;

  C_START = place
  C_END = C_START + mat_dim([a, c]) - 1
  out.newLine()
  out.comment("C' (" + str(a) + "x" + str(c) + "): $"
      + str(C_START) + " ... $" + str(C_END))

  out.call("matrixMul", [(C_START, C_END)], [(M_START, M_END), (N_START, N_END)])
  place += mat_dim([a, c])
  R_START = C_START

  C_START = place
  C_END = C_START + mat_dim([a, c]) - 1
  out.newLine()
  out.comment("C (" + str(a) + "x" + str(c) + "): $"
      + str(C_START) + " ... $" + str(C_END))

  out.new(C_START, C_END, True)
  for i in range(0, mat_dim([a, c])):
    out.public(place)
    place += 1;

  # Assert equal function.
  out.newLine()
  out.beginFunction("assertEq", [], [(0, 1), (0, 1)])
  out.mulc(2, 1, p - 1)
  out.add(3, 0, 2)
  out.assertZero(3)
  out.end()

  for i in range(0, mat_dim([a, c])):
    out.call("assertEq", [], [(R_START + i,), (C_START + i,)])

  out.end()

def generateMatrixProductIR0(ir, a, b, c, p, rel_file, ins_file, wit_file):
  # read the IR variant
  mem = False
  if ir in [ "flat_tb", "flat_pt", "dotprod_tb", "dotprod_pt", "plugin_pt", "dotprod_row_tb" ]:
    pass
  elif ir in [ "mem_flat_tb", "mem_flat_pt", "mem_dotprod_tb", "mem_dotprod_pt", "mem_plugin_pt", "mem_dotprod_row_tb" ]:
    ir = ir[4:]
    mem = True
  else:
    print("Unrecognized IR Variant: " + ir)
    sys.exit(1)

  # generate the headers
  if ir == "plugin_pt":
    header(rel_file, "circuit;\n@plugin wizkit_vectors", p)
  else:
    header(rel_file, "circuit", p)
  header(ins_file, "public_input", p)
  header(wit_file, "private_input", p)

  # generate the public/private input streams.
  generateInsWit(ins_file, wit_file, [a, b], [b, c], [a, c], p)

  out = tcg.GateWriter(rel_file, 0, True , not mem)

  # Write out the dot product function
  if ir == "dotprod_tb" or ir == "dotprod_row_tb":
    generateDotProdTB(out, b, c)
  elif ir == "dotprod_pt":
    generateDotProdPT(out, b)
    generatePartialTranspose(out, b, c)
  elif ir == "plugin_pt":
    generateDotProdPlugin(out, b)
    generatePartialTranspose(out, b, c)

  # write out the row helper function
  if ir == "dotprod_row_tb":
    generateRowTB(out, b, c)

  # Write out the matrix-multiplier function
  generateMatrixMul(out, ir, a, b, c)

  generateMainBody(out, a, b, c, p)

  rel_file.flush()
  rel_file.close()

  ins_file.flush()
  ins_file.close()

  wit_file.flush()
  wit_file.close()

if __name__ == "__main__":
  if len(sys.argv) != 9:
    print("Usage: matrix_prod <ir> <a> <b> <c> <p> <output>.rel <output>.ins <output>.wit")
    print("  for proving the product of M (axb, instance) and N (bxc, witness)")
    print("  is equal to C (axc, instance) over field p and encoded in <ir>.")
    print("  the following IRs are accepted:")
    print("   - flat_tb: flat circuit with textbook multiplier")
    print("   - flat_pt: flat circuit with partial transpose multiplier")
    print("   - dotprod_tb: dot product function with textbook multiplier")
    print("   - dotprod_pt: dot product function with partial transpose multiplier")
    print("   - plugin_pt: dot product via a plugin with partial transpose multiplier")
    print("   - dotprod_row_tb: dot product function with textbook multiplier, and a helper function for rows")
    print("   - add \"mem_\" prefix to aggressively manage memory")
    sys.exit(1)

  IR = sys.argv[1]
  
  A = int(sys.argv[2])
  B = int(sys.argv[3])
  C = int(sys.argv[4])
  P = int(sys.argv[5])

  REL = open(sys.argv[6], "w")
  INS = open(sys.argv[7], "w")
  WIT = open(sys.argv[8], "w")

  generateMatrixProductIR0(IR, A, B, C, P, REL, INS, WIT)
