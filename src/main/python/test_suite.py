#! /usr/bin/ python3

# Script to run the test suite. There should be a csv file of tests
# listing relation, instance, witness files to be tested together and
# whether they should succeed or fail.

import csv
import os
import subprocess as sp

# helper to delete a file.
def deleteFile(fname):
  try:
    if os.path.isfile(fname):
      os.remove(fname)
  except:
    pass

# converts a CSV expected result to a boolean
def result_col(col):
  col = str(col).strip()
  if col == "accept":
    return True
  elif col == "reject":
    return False
  else:
    print("invalid expected result:" + str(col))
    exit(2)

# converts a boolean to a test result string
def result_str(res):
  if res:
    return "accept"
  else:
    return "reject"

# some terminal colors
DEFAULT_COLOR = "\033[0m"
RED_COLOR = "\033[0;31m"
GREEN_COLOR = "\033[0;32m"
PURPLE_COLOR = "\033[0;35m"

# returns the ANSII color string for the result
def result_color(exp, res):
  if exp == res:
    return GREEN_COLOR
  else:
    return RED_COLOR

def print_result(expected, actual, name):
  print("    " + name + " expected: " + result_str(expected)
      + ", actual: " + result_color(expected, actual)
      + result_str(actual) + DEFAULT_COLOR)

def test_firealarm_1(resource, expected, name):
  args = ["target/wtk-firealarm", resource]
  actual = sp.run(args, stdout=sp.DEVNULL, stderr=sp.DEVNULL).returncode == 0
  print_result(expected, actual, name)
  if expected != actual:
    print("      command: " + " ".join(args));
  return expected != actual

def test_firealarm_3(relation, instance, witness, expected):
  args = ["target/wtk-firealarm", relation, instance, witness]
  actual = sp.run(args, stdout=sp.DEVNULL, stderr=sp.DEVNULL).returncode == 0
  print_result(expected, actual, "evaluate")
  if expected != actual:
    print("      command: " + " ".join(args));
  return expected != actual

def test_bolt(relation, instance, witness, expected):
  args = ["target/wtk-bolt", "bolt", relation, instance, witness]
  bolt_actual = sp.run(
      args, stdout=sp.DEVNULL, stderr=sp.DEVNULL).returncode == 0
  print_result(expected, bolt_actual, "bolt demo")
  if expected != bolt_actual:
    print("      command: " + " ".join(args));
  args = ["target/wtk-bolt", "plasmasnooze", relation, instance, witness]
  plasmasnooze_actual = sp.run(
      args, stdout=sp.DEVNULL, stderr=sp.DEVNULL).returncode == 0
  print_result(expected, plasmasnooze_actual, "plasmasnooze demo")
  if expected != plasmasnooze_actual:
    print("      command: " + " ".join(args));
  return expected != bolt_actual or expected != plasmasnooze_actual

def test_firealarm_1_irregular(resource, expected, name):
  args = ["target/wtk-firealarm", "-i", resource]
  actual = sp.run(args, stdout=sp.DEVNULL, stderr=sp.DEVNULL).returncode == 0
  print_result(expected, actual, name)
  if expected != actual:
    print("      command: " + " ".join(args));
  return expected != actual

def test_firealarm_3_irregular(relation, instance, witness, expected):
  args = ["target/wtk-firealarm", "-i", relation, instance, witness]
  actual = sp.run(args, stdout=sp.DEVNULL, stderr=sp.DEVNULL).returncode == 0
  print_result(expected, actual, "evaluate")
  if expected != actual:
    print("      command: " + " ".join(args));
  return expected != actual

def test_press(direction, relation, instance, witness,
        rel_expected, ins_expected, wit_expected, eval_expected, prefix):
  args_press_rel = ["target/wtk-press", direction, relation, prefix + ".rel"]
  press_rel_actual = sp.run(args_press_rel,
      stdout=sp.DEVNULL, stderr=sp.DEVNULL).returncode == 0
  args_firealarm_rel = []
  if direction.endswith("b"):
    args_firealarm_rel = ["target/wtk-firealarm", "-f", prefix + ".rel"]
  else:
    args_firealarm_rel = ["target/wtk-firealarm", prefix + ".rel"]
  print_result(True, press_rel_actual, "    press relation")
  firealarm_rel_actual = sp.run(args_firealarm_rel,
      stdout=sp.DEVNULL, stderr=sp.DEVNULL).returncode == 0
  print_result(rel_expected, firealarm_rel_actual, "firealarm relation")

  if not press_rel_actual or firealarm_rel_actual != rel_expected:
    print("      command: " + " ".join(args_press_rel))
    print("      command: " + " ".join(args_firealarm_rel))

  args_press_ins = ["target/wtk-press", direction, instance, prefix + ".ins"]
  press_ins_actual = sp.run(args_press_ins,
      stdout=sp.DEVNULL, stderr=sp.DEVNULL).returncode == 0
  args_firealarm_ins = []
  if direction.endswith("b"):
    args_firealarm_ins = ["target/wtk-firealarm", "-f", prefix + ".ins"]
  else:
    args_firealarm_ins = ["target/wtk-firealarm", prefix + ".ins"]
  print_result(True, press_ins_actual, "    press instance")
  firealarm_ins_actual = sp.run(args_firealarm_ins,
      stdout=sp.DEVNULL, stderr=sp.DEVNULL).returncode == 0
  print_result(ins_expected, firealarm_ins_actual, "firealarm instance")

  if not press_ins_actual or firealarm_ins_actual != ins_expected:
    print("      command: " + " ".join(args_press_ins))
    print("      command: " + " ".join(args_firealarm_ins))

  args_press_wit = ["target/wtk-press", direction, witness, prefix + ".wit"]
  press_wit_actual = sp.run(args_press_wit,
      stdout=sp.DEVNULL, stderr=sp.DEVNULL).returncode == 0
  args_firealarm_wit = []
  if direction.endswith("b"):
    args_firealarm_wit = ["target/wtk-firealarm", "-f", prefix + ".wit"]
  else:
    args_firealarm_wit = ["target/wtk-firealarm", prefix + ".wit"]
  print_result(True, press_wit_actual, "     press witness")
  firealarm_wit_actual = sp.run(args_firealarm_wit,
      stdout=sp.DEVNULL, stderr=sp.DEVNULL).returncode == 0
  print_result(wit_expected, firealarm_wit_actual, " firealarm witness")

  if not press_wit_actual or firealarm_wit_actual != wit_expected:
    print("      command: " + " ".join(args_press_wit))
    print("      command: " + " ".join(args_firealarm_wit))

  if press_rel_actual and press_ins_actual and press_wit_actual:
    args_firealarm_eval = []
    if direction.endswith("b"):
      args_firealarm_eval = ["target/wtk-firealarm", "-f", prefix + ".rel",
          prefix + ".ins", prefix + ".wit"]
    else:
      args_firealarm_eval = ["target/wtk-firealarm", prefix + ".rel",
          prefix + ".ins", prefix + ".wit"]
    firealarm_eval_actual = sp.run(args_firealarm_eval,
        stdout=sp.DEVNULL, stderr=sp.DEVNULL).returncode == 0
    print_result(eval_expected, firealarm_eval_actual, "firealarm evaluate")

    if firealarm_eval_actual != eval_expected:
      print("      command: " + " ".join(args_press_rel))
      print("      command: " + " ".join(args_press_ins))
      print("      command: " + " ".join(args_press_wit))
      print("      command: " + " ".join(args_firealarm_eval))
      return True
    else:
      return firealarm_rel_actual != rel_expected \
        or firealarm_ins_actual != ins_expected \
        or firealarm_wit_actual != wit_expected
  else:
    return True

def test_press_mux(mode, relation, instance, witness, eval_expected, prefix):
  args_press_mux = ["target/wtk-press", mode, relation, prefix + ".rel"]
  press_mux_actual = sp.run(args_press_mux,
          stdout=sp.DEVNULL, stderr=sp.DEVNULL).returncode == 0
  print_result(True, press_mux_actual, " press " + mode + " relation")
  if press_mux_actual:
    fail = False
    if mode == "t2mux":
      args_firealarm = ["target/wtk-firealarm", prefix + ".rel",
          instance, witness]
      firealarm_actual = sp.run(args_firealarm,
          stdout=sp.DEVNULL, stderr=sp.DEVNULL).returncode == 0
      print_result(eval_expected, firealarm_actual, "   firealarm evaluate")

      if firealarm_actual != eval_expected:
        print("      command: " + " ".join(args_press_mux))
        print("      command: " + " ".join(args_firealarm))
        fail = True

      args_firealarm_irregular = ["target/wtk-firealarm", "-i", prefix + ".rel",
          instance, witness]
      firealarm_irregular_actual = sp.run(args_firealarm_irregular,
          stdout=sp.DEVNULL, stderr=sp.DEVNULL).returncode == 0
      print_result(
          eval_expected, firealarm_irregular_actual, "firealarm -i evaluate")
      if firealarm_irregular_actual != eval_expected:
        print("      command: " + " ".join(args_press_mux))
        print("      command: " + " ".join(args_firealarm_irregular))
        fail = True
      args_bolt = ["target/wtk-bolt", "bolt", prefix + ".rel", instance,
          witness]
      bolt_actual = sp.run(args_bolt,
          stdout=sp.DEVNULL, stderr=sp.DEVNULL).returncode == 0
      print_result(eval_expected, bolt_actual, "        bolt evaluate")
      if bolt_actual != eval_expected:
        print("      command: " + " ".join(args_press_mux))
        print("      command: " + " ".join(args_bolt))
        fail = True
      args_plasmasnooze = ["target/wtk-bolt", "plasmasnooze", prefix + ".rel",
          instance, witness]
      plasmasnooze_actual = sp.run(args_plasmasnooze,
          stdout=sp.DEVNULL, stderr=sp.DEVNULL).returncode == 0
      print_result(eval_expected, plasmasnooze_actual, "plasmasnooze evaluate")
      if plasmasnooze_actual != eval_expected:
        print("      command: " + " ".join(args_press_mux))
        print("      command: " + " ".join(args_plasmasnooze))
        fail = True
    else:
      args_firealarm = ["target/wtk-firealarm", "-f", prefix + ".rel",
          instance, witness]
      firealarm_actual = sp.run(args_firealarm,
          stdout=sp.DEVNULL, stderr=sp.DEVNULL).returncode == 0
      print_result(eval_expected, firealarm_actual, "   firealarm evaluate")

      if firealarm_actual != eval_expected:
        print("      command: " + " ".join(args_press_mux))
        print("      command: " + " ".join(args_firealarm))
        fail = True
    return fail
  else:
    print("      command: " + " ".join(args_press_mux))
    return True

def check_diff(file_a, file_b):
  args = ["diff", file_a, file_b]
  same = sp.run(args, stdout = sp.DEVNULL, stderr=sp.DEVNULL).returncode == 0
  if not same:
    print("    diff " + file_a + " " + file_b + ": "
        + RED_COLOR + "different" + DEFAULT_COLOR)
    return True
  else:
    print("    diff " + file_a + " " + file_b + ": "
        + GREEN_COLOR + "same" + DEFAULT_COLOR)
    return False

# CSV Column indexes
REL_RESULT_COL = 0
INS_RESULT_COL = 1
WIT_RESULT_COL = 2
EVAL_RESULT_COL = 3
REL_COL = 4
INS_COL = 5
WIT_COL = 6

tests = csv.reader(open("automated_tests.csv", "r"))

failed_tests = list()
test_count = 0;
for test in tests:
  test_fail = False
  test_count = test_count + 1
  rel_file = str(test[REL_COL]).strip()
  rel_expected = result_col(test[REL_RESULT_COL])
  ins_file = str(test[INS_COL]).strip()
  ins_expected = result_col(test[INS_RESULT_COL])
  wit_file = str(test[WIT_COL]).strip()
  wit_expected = result_col(test[WIT_RESULT_COL])
  eval_expected = result_col(test[EVAL_RESULT_COL])

  test_name = "rel: " + PURPLE_COLOR + rel_file + DEFAULT_COLOR \
      + " ins: " + PURPLE_COLOR + ins_file + DEFAULT_COLOR \
      + " wit: " + PURPLE_COLOR + wit_file + DEFAULT_COLOR
  print(test_name)

  if not os.path.isfile(rel_file):
    test_fail = True
    print(RED_COLOR + "relation file not found: " + rel_file + DEFAULT_COLOR)
    continue
  if not os.path.isfile(ins_file):
    test_fail = True
    print(RED_COLOR + "instance file not found: " + ins_file + DEFAULT_COLOR)
    continue
  if not os.path.isfile(wit_file):
    test_fail = True
    print(RED_COLOR + "witness file not found: " + wit_file + DEFAULT_COLOR)
    continue

  print("  wtk-firealarm")
  test_fail = test_firealarm_1(rel_file, rel_expected, "relation") or test_fail
  test_fail = test_firealarm_1(ins_file, ins_expected, "instance") or test_fail
  test_fail = test_firealarm_1(wit_file, wit_expected, " witness") or test_fail
  test_fail = test_firealarm_3(
      rel_file, ins_file, wit_file, eval_expected) or test_fail
  print("  wtk-firealarm -i")
  test_fail = test_firealarm_1_irregular(
      rel_file, rel_expected, "relation") or test_fail
  test_fail = test_firealarm_1_irregular(
      ins_file, ins_expected, "instance") or test_fail
  test_fail = test_firealarm_1_irregular(
      wit_file, wit_expected, " witness") or test_fail
  test_fail = test_firealarm_3_irregular(
      rel_file, ins_file, wit_file, eval_expected) or test_fail
  print("  wtk-bolt")
  test_fail = test_bolt(
      rel_file, ins_file, wit_file, eval_expected) or test_fail
  print("  wtk-press t2t")
  test_fail = test_press("t2t", rel_file, ins_file, wit_file,
      rel_expected, ins_expected, wit_expected, eval_expected,
      "test-press.t2t") or test_fail
  print("  wtk-press t2b")
  test_fail = test_press("t2b", rel_file, ins_file, wit_file,
      rel_expected, ins_expected, wit_expected, eval_expected,
      "test-press.t2b") or test_fail
  print("  wtk-press b2t")
  test_fail = test_press(
      "b2t", "test-press.t2b.rel", "test-press.t2b.ins", "test-press.t2b.wit",
      rel_expected, ins_expected, wit_expected, eval_expected,
      "test-press.b2t") or test_fail
  print("  wtk-press b2b")
  test_fail = test_press(
      "b2b", "test-press.t2b.rel", "test-press.t2b.ins", "test-press.t2b.wit",
      rel_expected, ins_expected, wit_expected, eval_expected,
      "test-press.b2b") or test_fail

  print("  wtk-press diffs")
  test_fail = check_diff("test-press.t2t.rel", "test-press.b2t.rel") or test_fail
  test_fail = check_diff("test-press.t2t.ins", "test-press.b2t.ins") or test_fail
  test_fail = check_diff("test-press.t2t.wit", "test-press.b2t.wit") or test_fail
  test_fail = check_diff("test-press.t2b.rel", "test-press.b2b.rel") or test_fail
  test_fail = check_diff("test-press.t2b.ins", "test-press.b2b.ins") or test_fail
  test_fail = check_diff("test-press.t2b.wit", "test-press.b2b.wit") or test_fail

  print("  wtk-press t2mux")
  test_fail = test_press_mux("t2mux", rel_file, ins_file, wit_file,
          eval_expected, "test-press.t2mux") or test_fail
  print("  wtk-press b2mux")
  test_fail = test_press_mux("b2mux", "test-press.b2b.rel",
          "test-press.b2b.ins", "test-press.b2b.wit",
          eval_expected, "test-press.b2mux") or test_fail

  deleteFile("test-press.t2t.rel")
  deleteFile("test-press.t2t.ins")
  deleteFile("test-press.t2t.wit")
  deleteFile("test-press.t2b.rel")
  deleteFile("test-press.t2b.ins")
  deleteFile("test-press.t2b.wit")
  deleteFile("test-press.b2t.rel")
  deleteFile("test-press.b2t.ins")
  deleteFile("test-press.b2t.wit")
  deleteFile("test-press.b2b.rel")
  deleteFile("test-press.b2b.ins")
  deleteFile("test-press.b2b.wit")
  deleteFile("test-press.t2mux.rel")
  deleteFile("test-press.b2mux.rel")
  if(test_fail):
    failed_tests.append(test_name)

if 0 != len(failed_tests):
  print(RED_COLOR + "Failed " + str(len(failed_tests)) + " tests of " \
      + str(test_count) + DEFAULT_COLOR)
  for test in failed_tests:
    print("  " + test)
  exit(1)
else:
  print(GREEN_COLOR + "All " + str(test_count) + " tests passed" \
      + DEFAULT_COLOR)
  exit(0)
