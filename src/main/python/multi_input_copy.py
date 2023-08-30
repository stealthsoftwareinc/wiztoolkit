#! /usr/bin/python3

# Copyright (C) 2023, Stealth Software Technologies, Inc.

# This script will generate an IR statement for testing that the multi-wire
# copy and input gates work as intended.

import sys
import random
import TestCaseGen as tcg

def streams(ins, wit, p):
  ins.write("version 2.1.0;\n")
  wit.write("version 2.1.0;\n")
  ins.write("public_input;\n")
  wit.write("private_input;\n")
  ins.write("@type field " + str(p) + ";\n")
  wit.write("@type field " + str(p) + ";\n")
  ins.write("@begin\n")
  wit.write("@begin\n")

  for i in range(15):
    z = random.randrange(0, p)
    ins.write(" < " + str(z) + " > ; \n")
    wit.write(" < " + str(z) + " > ; \n")

  ins.write("@end\n")
  wit.write("@end\n")

  ins.flush()
  wit.flush()
  ins.close()
  wit.close()

def relation(f, p):
  f.write("version 2.1.0;\n")
  f.write("circuit;\n")
  f.write("@type field " + str(p) + ";\n")
  f.write("@begin\n")
  f.write("@function(assert_equal, @in: 0:1, 0:1)\n")
  f.write("  $2<-@mulc(0:$1, <" + str(p - 1) + ">);\n")
  f.write("  $3<-@add(0:$0, $2);\n")
  f.write("  @assert_zero(0:$3);\n")
  f.write("@end\n\n");

  f.write(" $0 ... $9 <- @private ( ) ; \n")
  f.write(" $10 ... $19 <- @public ( ) ; \n")
  f.write(" $20 ... $24 <- @private ( 0 ) ; \n")
  f.write(" $25 ... $29 <- @public(0);\n")
  f.write(" $30 ... $34 <- $0 , $3 , $1 , $2 , $4 ; \n")
  f.write(" $35 ... $39 <- 0 : $10 , $13 , $11 , $12 , $14 ; \n")
  f.write(" $40 ... $54 <- $5 ... $9, $20 ... $24, $30 ... $34 ; \n")
  f.write(" $55 ... $69 <- $15 ... $19, $25 ... $29, $35 ... $39 ; \n")

  for i in range(40, 55):
    f.write(" @call ( assert_equal , $" + str(i) + " , $" + str(i + 15) \
        + " ) ; \n")
  f.write("@end\n")

  f.flush()
  f.close()

if __name__ == "__main__":
  if len(sys.argv) != 3:
    print("USAGE: multi_input_copy <prime> <output>\n")
    print("Generate a test circuit for the multi-wire input and copy gates.")
    print("  prime: the prime field.")
    print("  output: the basename for created files.")

  prime = int(sys.argv[1])
  output = str(sys.argv[2])

  rel = open(output + ".rel", "w")
  ins = open(output + ".ins", "w")
  wit = open(output + ".wit", "w")

  relation(rel, prime)
  streams(ins, wit, prime)
