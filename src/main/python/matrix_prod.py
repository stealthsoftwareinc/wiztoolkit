#! /usr/bin/python3

# Copyright (C) 2021 Stealth Software Technologies, Inc.

import sys
import random
import numpy as np

if len(sys.argv) != 9:
  print("Usage: matrix_mult [ir0|ir1] a b c p <output>.rel <output>.ins <output>.wit")
  print("  for the product of M (axb, instance) and N (bxc, witness)")
  print("  is equal to C (axc, instance) over field p")
  sys.exit(1)

ir = sys.argv[1]
if ir == "ir0":
  ir = 0
elif ir == "ir1":
  ir = 1
else:
  print("bad ir " + ir)
  sys.exit(1)

a = int(sys.argv[2])
b = int(sys.argv[3])
c = int(sys.argv[4])
p = int(sys.argv[5])

# output some gate counts
main_loops = a * c
inner_loop_muls = b * main_loops
sum_add = (b - 1) * main_loops
check = a * c

print("add gates:     " + str(sum_add + check))
print("mul gates:     " + str(inner_loop_muls))
print("mulc gates:    " + str(check))
print("assert zeroes: " + str(check))
print("instances:     " + str(a * b + a * c))
print("witnesses:     " + str(b * c))
print("total:         " + str(sum_add + check * 3 + inner_loop_muls + a * b + a * c + b * c))

# Generate instance/witness random matrices.
M = np.ndarray((a, b), dtype=object)
for i in range(0, a):
  for j in range(0, b):
    M[i][j] = random.randint(0, p)
N = np.ndarray((b, c), dtype=object)
for i in range(0, b):
  for j in range(0, c):
    N[i][j] = random.randint(0, p)

def mod(x):
  return x % p

# generate N*M product
C = mod(np.matmul(M, N, dtype=object))

relation = open(sys.argv[6], "w")
instance = open(sys.argv[7], "w")
witness = open(sys.argv[8], "w")

relation.write("version 1.0.0;\n")
instance.write("version 1.0.0;\n")
witness.write("version 1.0.0;\n")
relation.write("field characteristic " + str(p) + " degree 1;\n")
instance.write("field characteristic " + str(p) + " degree 1;\n")
witness.write("field characteristic " + str(p) + " degree 1;\n")

relation.write("relation\n")
relation.write("gate_set:arithmetic;\n")
instance.write("instance\n@begin\n")
witness.write("short_witness\n@begin\n")

if(ir == 0):
  relation.write("features:simple;\n")
else:
  relation.write("features:@for,@function;\n")

relation.write("@begin\n")

if ir == 1:
  # write out the sum function.
  relation.write("\n@function(sum, @out: 1, @in: " + str(b) \
      + ", @instance: 0, @short_witness: 0)\n")
  relation.write("  // out: $0\n")
  relation.write("  // in: $1 ... $" + str(b) + "\n")
  relation.write("  $" + str(1 + b) + " <- @add($1, $2);\n")
  relation.write("  $" + str(2 + b) + " ... $" + str(b + b - 2) \
      + " <- @for i @first 2 @last " + str(b - 2) + "\n")
  relation.write("    $(i + " + str(b) \
      + ") <- @anon_call($(1 + i), $(" + str(b - 1) \
      + " + i), @instance: 0, @short_witness: 0)\n")
  relation.write("      $0 <- @add($1, $2);\n")
  relation.write("    @end\n")
  relation.write("  @end\n")
  relation.write("  $0 <- @add($" + str(b) + ", $" + str(b + b - 2) + ");\n")
  relation.write("@end\n")

instance_M_start = 0
instance_M_end = instance_M_start + a * b
instance_C_start = instance_M_end
instance_C_end = instance_C_start + a * c
witness_start = instance_C_end
witness_end = witness_start + b * c

C_prime_start = witness_end
C_prime_end = C_prime_start + a * c

num_wires = C_prime_end + (a * c * b) + (a * c * (b - 2)) + a * c * 2

relation.write("\n// M (" + str(a) + "x" + str(b) + "): $" \
    + str(instance_M_start) + "...$" + str(instance_M_end - 1) + " (instance)")
relation.write("\n// N (" + str(b) + "x" + str(c) + "): $" \
    + str(witness_start) + "...$" + str(witness_end - 1) + " (witness)")
relation.write("\n// C (" + str(a) + "x" + str(c) + "): $" \
    + str(instance_C_start) + "...$" + str(instance_C_end - 1) + " (instance)")
relation.write("\n// C' (" + str(a) + "x" + str(c) + "): $" \
    + str(C_prime_start) + "...$" + str(C_prime_end - 1) + "\n\n")

relation.write("// Computes the product C':=M*N, and checks that C'==C\n\n")

# write out the instance and witness
next_wire = 0

for i in range(0, len(M)):
  for j in range(0, len(M[i])):
    if ir == 0:
      relation.write("$" + str(next_wire) + "<-@instance;\n")
      next_wire = next_wire + 1
    instance.write("<" + str(M[i][j]) + ">;\n")

for i in range(0, len(C)):
  for j in range(0, len(C[i])):
    if ir == 0:
      relation.write("$" + str(next_wire) + "<-@instance;\n")
      next_wire = next_wire + 1
    instance.write("<" + str(C[i][j]) + ">;\n")

for i in range(0, len(N)):
  for j in range(0, len(N[i])):
    if ir == 0:
      relation.write("$" + str(next_wire) + "<-@short_witness;\n")
      next_wire = next_wire + 1
    witness.write("<" + str(N[i][j]) + ">;\n")

if ir == 1:
  # M is instance
  first = str(next_wire)
  last = str(next_wire + len(M) * len(M[0]) - 1)
  relation.write("$" + first + " ... $" + last + " <- @for i @first " \
      + first + " @last " + last + "\n")
  relation.write("  $i <- @anon_call(@instance: 1, @short_witness: 0)\n")
  relation.write("    $0 <- @instance;\n")
  relation.write("  @end\n")
  relation.write("@end\n")
  next_wire = next_wire + len(M) * len(M[0])
  # C is instance
  first = str(next_wire)
  last = str(next_wire + len(C) * len(C[0]) - 1)
  relation.write("$" + first + " ... $" + last + " <- @for i @first " \
      + first + " @last " + last + "\n")
  relation.write("  $i <- @anon_call(@instance: 1, @short_witness: 0)\n")
  relation.write("    $0 <- @instance;\n")
  relation.write("  @end\n")
  relation.write("@end\n")
  next_wire = next_wire + len(C) * len(C[0])
  # N is witness
  first = str(next_wire)
  last = str(next_wire + len(N) * len(N[0]) - 1)
  relation.write("$" + first + " ... $" + last + " <- @for i @first " \
      + first + " @last " + last + "\n")
  relation.write("  $i <- @anon_call(@instance: 0, @short_witness: 1)\n")
  relation.write("    $0 <- @short_witness;\n")
  relation.write("  @end\n")
  relation.write("@end\n\n")
  next_wire = next_wire + len(N) * len(N[0])

instance.write("@end\n")
witness.write("@end\n")

# write out the relation
next_wire = C_prime_end

if ir == 0:
  for i in range(0, a):
    for j in range(0, c):
      sum_start = next_wire
      relation.write("// C'[" + str(i) + "," + str(j) + \
          "] <- SUM_k M[" + str(i) + ", k] * N[k, " + str(j) + "]\n")
      for k in range(0, b):
        relation.write("$" + str(next_wire) + "<-@mul($" + \
            str(instance_M_start + (i * b) + k) + ",$" + \
            str(witness_start + (k * c) + j) + ");\n");
        next_wire = next_wire + 1
      sum_end = next_wire
      for k in range(sum_start + 1, sum_end):
        if k == sum_end - 1:
          relation.write("$" + str(C_prime_start + (i * c) + j))
        else:
          relation.write("$" + str(next_wire))
          next_wire = next_wire + 1
        relation.write("<-@add(");
        if k == sum_start + 1:
          relation.write("$" + str(sum_start))
        elif k == sum_end - 1:
          relation.write("$" + str(next_wire - 1))
        else:
          relation.write("$" + str(next_wire - 2))
        relation.write(",$" + str(k) + ");\n")
      relation.write("@delete($" + str(sum_start) + ",$" + str(next_wire - 1) + ");\n")
else:
  relation.write("$" + str(C_prime_start) + " ... $" +str(C_prime_end - 1) \
      + " <- @for i @first  0 @last " + str(a - 1) + "\n")
  relation.write("  $(" + str(C_prime_start) + " + (i * " + str(c) \
      + ")) ... $(" + str(C_prime_start + c - 1) + " + (i * " + str(c) \
      + ")) <- @anon_call($" + str(instance_M_start) + " ... $" \
      + str(instance_M_end - 1) + ", $" + str(witness_start) \
      + " ... $" + str(witness_end - 1) \
      + ", @instance: 0, @short_witness: 0)\n")
  relation.write("    // C'[i][...]: $0 ... $" + str(c - 1) + "\n")
  relation.write("    // M: $" + str(c) + " ... $" + str(c + len(M) * len(M[0]) - 1) + "\n")
  relation.write("    // N: $" + str(c + len(M) * len(M[0]))  + " ... $" \
      + str(c + len(M) * len(M[0]) + len(N) * len(N[0]) - 1) + "\n")
  relation.write("    $0 ... $" + str(c - 1) + " <- @for j @first 0 @last " \
      + str(c - 1) + "\n")
  relation.write("      $j <- @anon_call($" + str(c) + " ... $" \
      + str(c + len(M) * len(M[0]) - 1) + ", $" + str(c + len(M) * len(M[0])) + "... $" \
      + str(c + len(M) * len(M[0]) + len(N) * len(N[0]) - 1) + ", @instance: 0, @short_witness: 0)\n")
  relation.write("        // C'[i][j]: $0\n")
  relation.write("        // M: $1 ... $" + str(len(M) * len(N[0])) + "\n")
  relation.write("        // N: $" + str(1 + len(M) * len(M[0])) + " ... $" \
      + str(len(M) * len(M[0]) + len(N) * len(N[0])) + "\n")
  relation.write("        $" + str(1 + len(M) * len(M[0]) + len(N) * len(N[0])) + " ... $" \
      + str(b + len(M) * len(M[0]) + len(N) * len(N[0])) + " <- @for k @first 0 @last " + str(b - 1) \
      + "\n")
  relation.write("          $(k + " + str(1 + len(M) * len(M[0]) + len(N) * len(N[0])) \
      + ") <- @anon_call($(1 + ((i * " + str(b) + ") + k)), $(" \
      + str(1 + len(M) * len(M[0])) + " + ((k * " + str(c) + ") + j)), @instance: 0," \
      + "@short_witness: 0)\n")
  relation.write("            $0 <- @mul($1, $2);\n")
  
  relation.write("          @end\n")
  relation.write("        @end\n")
  relation.write("        $0 <- @call(sum, $" + str(1 + len(M) * len(M[0]) + len(N) * len(N[0])) \
      + " ... $" + str(b + len(M) * len(M[0]) + len(N) * len(N[0])) + ");\n")
  relation.write("      @end\n")
  relation.write("    @end\n")
  relation.write("  @end\n")
  relation.write("@end\n\n")

relation.write("// Check that C' == C\n")
if ir == 0:
  for i in range(0, a):
    del_wire = next_wire;
    for j in range(0, c):
      relation.write("$" + str(next_wire) + "<-@mulc($" \
          + str(instance_C_start + i * c + j) + ",<" \
          + str(p - 1) + ">);\n")
      relation.write("$" + str(next_wire + 1) + "<-@add($" \
          + str(C_prime_start + i * c + j) + ",$" \
          + str(next_wire) + ");\n")
      relation.write("@assert_zero($" + str(next_wire + 1) + ");\n")
      next_wire = next_wire + 2
    relation.write("@delete($" + str(del_wire) + ",$" + str(next_wire - 1) + ");\n")
else:
  relation.write("@for i @first 0 @last " + str(a - 1) + "\n")
  relation.write("  @anon_call($((i * " + str(c) + ") + " \
      + str(instance_C_start) + ") ...$((i * " + str(c) + ") + " \
      + str(instance_C_start + c - 1) + "), $((i * " + str(c) + ") + " \
      + str(C_prime_start) + ") ... $((i * " + str(c) + ") + " \
      + str(C_prime_start + c - 1) \
      + "), @instance: 0, @short_witness: 0)\n")
  relation.write("    // C[i][...]: $0 ... $" + str(c - 1) + "\n")
  relation.write("    // C'[i][...]: $" + str(c) + " ... $" + str(c + c - 1) \
      + "\n")
  relation.write("    @for j @first 0 @last " + str(c - 1) + "\n")
  relation.write("      @anon_call($j, $(j + " + str(c) \
      + "), @instance: 0, @short_witness: 0)\n")
  relation.write("        $2 <- @mulc($0, < " + str(p - 1) + " >);\n")
  relation.write("        $3 <- @add($1, $2);\n")
  relation.write("        @assert_zero($3);\n")
  relation.write("      @end\n")
  relation.write("    @end\n")
  relation.write("  @end\n")
  relation.write("@end\n\n")

relation.write("@end\n");

sys.exit(0)
