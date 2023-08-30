#!/usr/env/bin/python3

# Run some regression tests for wiztoolkit

import sys
import subprocess as sp
from pathlib import Path
import random
import threading
import shutil

import matrix_prod as mat_prod
import multiplexer_1type as mux1
import multiplexer_2type as mux2
import multiplexer_bool_type as muxbool
import memchk
import memchk_bool
import less_than_div_test as cmp_div
import multi_input_copy

CMD_DIR = "target/" if len(sys.argv) == 1 else sys.argv[1]
FIREALARM_CMD = CMD_DIR + "wtk-firealarm"
PRESS_CMD = CMD_DIR + "wtk-press"

# list of primes to use
primes = [ 2, 13, 127, 2**18 * 4095 + 1, 2**31 - 1, 2**61 - 1, 2**127 - 1 ]

# some terminal colors
DEFAULT_COLOR = "\033[0m"
RED_COLOR = "\033[0;31m"
GREEN_COLOR = "\033[0;32m"
PURPLE_COLOR = "\033[0;35m"

valgrind_cmd = shutil.which("valgrind")
has_valgrind = not valgrind_cmd is None

# super class for all the tests
class Test:
  def __init__(self):
    self.skip = False
    self.success = False
    self.valgrindRun = False
    self.valgrindSuccess = True
    self.flatbufferRun = False

  def runHelper(self, use_valgrind, program, args):
    if has_valgrind and use_valgrind:
      cmd_line = [ valgrind_cmd, "--leak-check=summary" ] + [ program ] + args
      grep_cmd = [ "grep", "ERROR SUMMARY: 0 errors" ]
      cmd_proc = sp.Popen(cmd_line, stdout=sp.PIPE, stderr=sp.STDOUT)
      grep_proc = sp.run(grep_cmd, stdin=cmd_proc.stdout, stdout=sp.DEVNULL)
      cmd_proc.wait()
      cmd_ok = cmd_proc.returncode == 0
      vg_ok = grep_proc.returncode == 0
      self.valgrindSuccess = self.valgrindSuccess and vg_ok
      self.valgrindRun = True
      if not(cmd_ok and vg_ok):
        print(RED_COLOR + "Failed Cmd: " + DEFAULT_COLOR + " ".join(cmd_line))
      self.success = self.success and cmd_ok
    else:
      cmd = [ program ] + args
      ok = 0 == sp.run(cmd, stdout=sp.DEVNULL, stderr=sp.DEVNULL).returncode
      if not ok:
        print(RED_COLOR + "Failed Cmd: " + DEFAULT_COLOR + " ".join(cmd))
      self.success = self.success and ok

  def run(self, basename):
    try:
      self.generateTestCase(basename)
    except Exception as e:
      print(PURPLE_COLOR + "Skipping Test: " + DEFAULT_COLOR + self.name())
      print(e)
      self.skip = True
    else:
      use_valgrind = random.randint(0, 250) == 0
      test_files = self.testFiles()
      self.success = True
      self.runHelper(use_valgrind, FIREALARM_CMD, test_files)
      if random.randint(0, 8) > 2:
        flatbuffer_files = []
        for f in test_files:
          self.runHelper(use_valgrind, PRESS_CMD, ["t2f", f, f + ".sieve"])
          flatbuffer_files.append(f + ".sieve")
        self.runHelper(use_valgrind, FIREALARM_CMD, ["-f"] + flatbuffer_files)
        retext_files = []
        for f in flatbuffer_files:
          self.runHelper(use_valgrind, PRESS_CMD, ["f2t", f, f + ".retxt"])
          retext_files.append(f + ".retxt")
        self.runHelper(use_valgrind, FIREALARM_CMD, retext_files)
        self.flatbufferRun = True

  def report(self):
    if not self.skip:
      if not self.success:
        print(RED_COLOR + "Failure: " + DEFAULT_COLOR + self.name())
      if self.valgrindRun:
        if not self.valgrindSuccess:
          print(RED_COLOR + "Valgrind Failure: " + DEFAULT_COLOR + self.name())
    return self.skip, self.success, self.valgrindRun, self.valgrindSuccess, self.flatbufferRun

# list of tests (append to, then run them)
tests = []

# ==== MATRIX PRODUCT ====

# make a test case from the matrix product script
class MatrixTest(Test):
  def __init__(self, prime, ir, a, b, c):
    super().__init__()
    self.prime = prime
    self.ir = ir
    self.a = a
    self.b = b
    self.c = c
    self.basename = ""

  def name(self):
    return "matrix " + self.ir + "(prime:" + str(self.prime) + ", a:" \
        + str(self.a) + ", b:" + str(self.b) + ", c:" + str(self.c) + ")"

  def generateTestCase(self, basename):
    self.basename = basename
    mat_prod.generateMatrixProductIR0( \
            self.ir, self.a, self.b, self.c, self.prime, \
            open(basename + ".rel", "w"), open(basename + ".ins", "w"), \
            open(basename + ".wit", "w"))

  def testFiles(self):
    return [ self.basename + ".rel", self.basename + ".ins", \
        self.basename + ".wit" ]

# matrix test parameters
matrix_small_sizes = [ 2, 3, 4, 5 ]
matrix_irs = [ "flat_tb", "mem_flat_tb", "flat_pt", "mem_flat_pt", \
    "dotprod_tb", "mem_dotprod_tb", "dotprod_pt", "mem_dotprod_pt", \
    "plugin_pt", "mem_plugin_pt", "dotprod_row_tb", "mem_dotprod_row_tb" ]

for ir in matrix_irs:
  for prime in primes:
    for a in matrix_small_sizes:
      for b in matrix_small_sizes:
        for c in matrix_small_sizes:
          tests.append(MatrixTest(prime, ir, a, b, c))

matrix_large_sizes = [ 10, 25, 50 ]

for ir in matrix_irs:
  for prime in primes:
    for s in matrix_large_sizes:
      tests.append(MatrixTest(prime, ir, s, s, s))

# ==== RAM MemChk ====

class MemChkTest(Test):
  def __init__(self, size, prime, plugin_v1 = False):
    super().__init__()
    self.size = size
    self.prime = prime
    self.basename = ""
    self.pluginV1 = plugin_v1

  def name(self):
    return "memchk(prime:" + str(self.prime) + ", size:" + str(self.size) + ")"

  def generateTestCase(self, basename):
    self.basename = basename
    names = self.testFiles()
    memchk.makeMemChkIR0(self.size, self.prime, \
        open(names[0], "w"), open(names[1], "w"), open(names[2], "w"), \
        self.pluginV1)

  def testFiles(self):
    return [ self.basename + ".rel", self.basename + ".ins", \
            self.basename + ".wit" ]

memchk_sizes = [ 2, 3, 4, 5, 6, 7, 8, 16, 32, 50, 99, 100, 101, 128 ]

for prime in primes:
  for size in memchk_sizes:
    if size <= prime:
      tests.append(MemChkTest(size, prime))
      tests.append(MemChkTest(size, prime, True))

# ==== RAM MemChk Bool ====

class MemChkBoolTest(Test):
  def __init__(self, size, idx_bits, elt_bits):
    super().__init__()
    self.size = size
    self.idxBits = idx_bits
    self.eltBits = elt_bits
    self.basename = ""

  def name(self):
    return "memchk bool(size:" + str(self.size) + ", idx_bits:" \
        + str(self.idxBits) + ", elt_bits:" + str(self.eltBits) + ")"

  def generateTestCase(self, basename):
    self.basename = basename
    names = self.testFiles()
    memchk_bool.makeMemChkIR0(self.size, self.idxBits, self.eltBits, \
        open(names[0], "w"), open(names[1], "w"), open(names[2], "w"))

  def testFiles(self):
    return [ self.basename + ".rel", self.basename + ".ins", \
            self.basename + ".wit" ]


memchk_bool_idx_bits = [3, 4, 8, 16, 17, 24 ]
memchk_bool_elt_bits = [2, 3, 4, 8, 9, 16, 17, 32, 35 ]
memchk_bool_sizes = [ 2, 3, 4, 5, 6, 7, 8, 16, 32, 50, 99, 100, 101, 128 ]

for idx_bits in memchk_bool_idx_bits:
  for elt_bits in memchk_bool_elt_bits:
    for size in memchk_sizes:
      if size < (1 << idx_bits):
        tests.append(MemChkBoolTest(size, idx_bits, elt_bits))


# ==== Bool Multiplexer 1-type Tests ====

class BoolMuxTest(Test):
  def __init__(self, ir, bits, cases, items):
    super().__init__()
    self.ir = ir
    self.bits = bits
    self.cases = cases
    self.items = items

  def name(self):
    return "muxbool " + self.ir + "(bits:" + str(self.bits) + ", cases:" \
        + str(self.cases) + ", items:" + str(self.items) + ")"

  def generateTestCase(self, basename):
    self.basename = basename
    names = self.testFiles()
    muxbool.generateBooleanMultiplexerIR0(self.ir, self.bits, self.cases, self.items,
        open(names[0], "w"), open(names[1], "w"), open(names[2], "w"))

  def testFiles(self):
    return [ self.basename + ".rel", self.basename + ".ins", \
            self.basename + ".wit" ]

muxbool_bits = [1, 2, 4, 5, 6]
muxbool_cases = [2, 3, 4, 8, 15, 16, 32, 36, 64]
muxbool_items = [2, 3, 4, 5, 6, 7, 8, 16, 32, 64, 128, 256]

for bits in muxbool_bits:
  for cases in muxbool_cases:
    if 2**bits >= cases:
      for items in muxbool_items:
        tests.append(BoolMuxTest("no_plugins", bits, cases, items))
        tests.append(BoolMuxTest("mux_v0", bits, cases, items))


# ==== Multiplexer 1-type Tests ====

class Mux1Test(Test):
  def __init__(self, ir, prime, cases, items):
    super().__init__()
    self.ir = ir
    self.prime = prime
    self.cases = cases
    self.items = items

  def name(self):
    return "mux1 " + self.ir + "(prime:" + str(self.prime) + ", cases:" \
        + str(self.cases) + ", items:" + str(self.items) + ")"

  def generateTestCase(self, basename):
    self.basename = basename
    names = self.testFiles()
    mux1.generateMultiplexerIR0(self.ir, self.prime, self.cases, self.items,
        open(names[0], "w"), open(names[1], "w"), open(names[2], "w"))

  def testFiles(self):
    return [ self.basename + ".rel", self.basename + ".ins", \
            self.basename + ".wit" ]

mux1_cases = [2, 3, 4, 8, 16, 32, 64]
mux1_items = [2, 3, 4, 5, 6, 7, 8, 16, 32, 64, 128, 256]

for prime in primes:
  for cases in mux1_cases:
    for items in mux1_items:
      if cases <= prime:
        tests.append(Mux1Test("no_plugins", prime, cases, items))
        tests.append(Mux1Test("mux_v0", prime, cases, items))

# ==== Multiplexer 2-type Tests ====

class Mux2Test(Test):
  def __init__(self, f_prime, g_prime, cases, items):
    super().__init__()
    self.fPrime = f_prime
    self.gPrime = g_prime
    self.cases = cases
    self.items = items

  def name(self):
      return "mux2(f_prime:" + str(self.fPrime) + ", g_prime:" \
          + str(self.gPrime) + ", cases:" + str(self.cases) + ", items:" \
          + str(self.items) + ")"

  def generateTestCase(self, basename):
    self.basename = basename
    names = self.testFiles()
    mux2.generateMultiplexerIR0(self.fPrime, self.gPrime, self.cases,
        self.items, open(names[0], "w"), open(names[1], "w"),
        open(names[2], "w"), open(names[3], "w"), open(names[4], "w"))

  def testFiles(self):
    return [ self.basename + ".rel", self.basename + ".F.ins", \
            self.basename + ".F.wit", self.basename + ".G.ins", \
            self.basename + ".G.wit" ]

mux2_cases = [2, 3, 4, 8, 16, 32, 64]
mux2_items = [2, 3, 4, 5, 6, 7, 8, 16, 32, 64, 128, 256]

for fprime in primes:
  for gprime in primes:
    for cases in mux2_cases:
      for items in mux2_items:
        if cases <= fprime and fprime != gprime:
          tests.append(Mux2Test(fprime, gprime, cases, items))

# ==== Comparison and Division Plugin Tests ====

class CmpDivTest(Test):
  def __init__(self, prime):
    super().__init__()
    self.prime = prime

  def name(self):
    return "div_cmp(prime:" + str(self.prime) + ")"

  def generateTestCase(self, basename):
    self.basename = basename
    names = self.testFiles()
    cmp_div.makeLessThanDivTestIR0(self.prime,
            open(names[0], "w"), open(names[1], "w"), open(names[2], "w"))

  def testFiles(self):
    return [ self.basename + ".rel", \
            self.basename + ".ins", \
            self.basename + ".wit", ]

for prime in primes[2:]:
  tests.append(CmpDivTest(prime))

# ==== Multi Input and Copy Tests ====

class MultiInputCopyTest(Test):
  def __init__(self, prime):
    super().__init__()
    self.prime = prime

  def name(self):
    return "multi_input_copy(prime:" + str(self.prime) + ")"

  def generateTestCase(self, basename):
    self.basename = basename
    names = self.testFiles()
    multi_input_copy.relation(open(names[0], "w"), self.prime)
    multi_input_copy.streams(open(names[1], "w"), open(names[2], "w"), self.prime)

  def testFiles(self):
    return [ self.basename + ".rel", \
            self.basename + ".ins", \
            self.basename + ".wit", ]

for prime in primes[2:]:
  tests.append(MultiInputCopyTest(prime))

# ==== RUN THE TESTS ====

Path("target/regression_tests").mkdir(parents=True, exist_ok=True)

i = 0
lock = threading.Lock()

print(str(len(tests)) + " tests")

def runTests():
  global tests
  global i

  while i < len(tests):
    lock.acquire()
    test = tests[i]
    i = i + 1
    lock.release()
    test.run("target/regression_tests/test" + str(i))

threads = []
for i in range(0, 4):
  thread = threading.Thread(target=runTests)
  threads.append(thread)
  thread.start()

for thread in threads:
  thread.join()

skip_count = 0
success_count = 0
failure_count = 0
valgrind_count = 0
valgrind_fails = 0
flatbuffer_count = 0
flatbuffer_fails = 0
for test in tests:
  skip, success, vg_run, vg_success, fb_run = test.report()
  if skip:
    skip_count += 1
  else:
    if success:
      success_count += 1
    else:
      failure_count += 1
    if vg_run:
      valgrind_count += 1
      if not vg_success:
        valgrind_fails += 1
    if fb_run:
      flatbuffer_count += 1

retcode = 0
print("Tests:   " + PURPLE_COLOR + str(len(tests)) + DEFAULT_COLOR)
print("Success: " + GREEN_COLOR + str(success_count) + DEFAULT_COLOR)
if skip_count != 0:
  print("Skips: " + PURPLE_COLOR + str(skip_count) + DEFAULT_COLOR)
if failure_count != 0:
  print("Failure: " + RED_COLOR + str(failure_count) + DEFAULT_COLOR)
  retcode = 1
print("Valgrind tests: " + PURPLE_COLOR + str(valgrind_count) + DEFAULT_COLOR)
if valgrind_fails != 0:
  print("Valgrind failures: " + RED_COLOR + str(valgrind_fails) + DEFAULT_COLOR)
  retcode = 1
print("Flatbuffer tests: " + PURPLE_COLOR + str(flatbuffer_count) + DEFAULT_COLOR)
if flatbuffer_fails != 0:
  print("Flatbuffer failures: " + RED_COLOR + str(flatbuffer_fails) + DEFAULT_COLOR)
  retcode = 1

sys.exit(retcode)
