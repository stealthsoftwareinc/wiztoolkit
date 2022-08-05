# Notes Regarding WizToolKit Development and Stability
WizToolKit is under constant development and attempts to track the SIEVE IR Specification.
As such, some APIs are subject to change.
If an API is to be considered "stable" it is stable only until the IR changes, at which point it may change, either to mirror the IR, or for other API improvements
This document will attempt to explain API changes and declare the status of various APIs.

## IR Version 1.0.0
The following APIs are stable.
Links currently go to header-files with documentation-like commentary. More recently, a [manual](https://stealthsoftwareinc.github.io/wiztoolkit/single_page_manual.html) has been added.

 - [``wtk/Parser.h``](src/main/cpp/wtk/Parser.h): defines the top level API for parsing.
   Its split along two boundaries, one for the gate set (arithmetic or boolean) and one for the parsing method (streaming or syntax-tree).
   This file defines an abstract API, with additional APIs defining drop-in replacement implementations.
    - The following three implementations exist or are planned to exist.
       - [``wtk/antlr/Parser.h``](src/main/cpp/wtk/antlr/Parser.h): An ANTLR implementation of the text format (complete).
       - [``wtk/irregular/Parser.h``](src/main/cpp/wtk/irregular/Parser.h): A hand written DFA based implementation of the text format (complete).
         - The advantage over ANTLR is speed and memory usage, at the cost of poor error recovery and no line-numbers.
       - [``wtk/flatbuffer/Parser.h``](src/main/cpp/wtk/flatbuffer/Parser.h): A FlatBuffer implementation (complete).
    - The following APIs are "helpers" to the Parser API.
       - [``wtk/IRParameters.h``](src/main/cpp/wtk/IRParameters.h) In memory representation for Header, Gate Set and other parameters of the IR.
       - [``wtk/IRTree.h``](src/main/cpp/wtk/IRTree.h) Abstract definiton of the in memory syntax tree. Each parser will provide an implementation.
       - [``wtk/ArithmeticStreamHandler.h``](src/main/cpp/wtk/ArithmeticStreamHandler.h) and [``wtk/BooleanStreamHandler.h``](src/main/cpp/wtk/BooleanStreamHandler.h): Abstract interfaces to the simple mode ("IR0") streaming API using flatbuffers.
         TA2 may implement this abstract class, and a parser will invoke methods in the sequence dictated by a relation.
 - For serializing text:
   - [``wtk/printers/printTextParameters.h``](src/main/cpp/wtk/printers/printTextParameters.h) Helper functions to print out IR Parameters and front matter (``wtk/IRParameters.h``).
   - [``wtk/printers/TextTreePrinter.h``](src/main/cpp/wtk/printers/TextTreePrinter.h) For serializing syntax trees (``wtk/IRTree.h``) in the text format.
   - [``wtk/printers/ArithmeticTextStreamPrinter.h``](src/main/cpp/wtk/printers/ArithmeticTextStreamPrinter.h) and [``wtk/printers/BooleanTextStreamPrinter.h``](src/main/cpp/wtk/printers/BooleanTextStreamPrinter.h) For serializing relations in text with the Streaming Parser API.
   - [``wtk/printers/printTextInputStream.h``](src/main/cpp/wtk/printers/printTextInputStream.h) For serializing instance and short witness resources in the text format.
 - For serializing FlatBuffers:
   - [``wtk/flatbuffer/FlatBufferPrinter.h``](src/main/cpp/wtk/flatbuffer/FlatBufferPrinter.h) abstract class for handling IR header and front matter stuff (``wtk/IRParameters.h``) for the other flatbuffer printers.
     This means that the flatbuffer serialization API differs from text in that front matter is passed to the class instead of serialized ahead of time.
   - [``wtk/flatbuffer/FlatBufferTreePrinter.h``](src/main/cpp/wtk/flatbuffer/FlatBufferTreePrinter.h) For serializing syntax trees (``wtk/IRTree.h``) using flatbuffers.
   - [``wtk/flatbuffer/ArithmeticFlatBufferStreamPrinter.h``](src/main/cpp/wtk/flatbuffer/ArithmeticFlatBufferStreamPrinter.h) and [``wtk/flatbuffer/BooleanFlatBufferStreamPrinter.h``](src/main/cpp/wtk/flatbuffer/BooleanFlatBufferStreamPrinter.h) For serializing relations as flatbuffers with the Streaming Parser API.
   - [``wtk/flatbuffer/FlatBufferInputStreamPrinter.h``](src/main/cpp/wtk/flatbuffer/FlatBufferInputStreamPrinter.h) For serializing instance and short witness resources in the flatbuffer format.

The following APIs are somewhat stable, but I can't promise they won't change.
 - [``wtk/utils/``](src/main/cpp/wtk/utils) These are helper functions and classes.
   Some are pretty generic while others are SIEVE IR specific.
 - [``wtk/firealarm/TreeAlarm.h``](src/main/cpp/wtk/firealarm/TreeAlarm.h) This is the implementation of the ``wtk-firealarm`` tool.
   It traverses the IR syntax tree (``wtk/IRTree.h``) to check for spec-compliance and check if evaluation is successful (without actual ZK).
   Most methods are virtual, incase extensions of this might be useful, but it may also be used as a validity-predicate for relations.
   The following helper classes are used.
    - [``wtk/firealarm/State.h``](src/main/cpp/wtk/firealarm/State.h) This implements the ``state`` abstraction from section 4.2 of the spec.
    - [``wtk/firealarm/WireSet.h``](src/main/cpp/wtk/firealarm/WireSet.h) This implements the ``wireset`` abstraction from section 4.2.1 of the spec.
 - [``wtk/firealarm/TraceTreeAlarm.h``](src/main/cpp/wtk/firealarm/TraceTreeAlarm.h) is a subclass of ``wtk/firealarm/TreeAlarm.h`` which prints out an execution trace.
 - [``wtk/viz/TreeVisualizer.h``](src/main/cpp/wtk/viz/TreeVisualizer.h) translates a relation to Graphviz.
 - [``wtk/converters/Multiplex.h``](src/main/cpp/wtk/converters/Multiplex.h) This takes a syntax tree and produces another syntax tree removed of switch-case statements using multiplexor logic implemented with for-loops and function-gates.
 - `wtk::bolt` is a collection of interpreters for the IR which utilize a pluggable ZK Backend API.
   - [``wtk/bolt/Backend.h``](src/main/cpp/wtk/bolt/Backend.h) An abstract class for ZK Backends to implement.
   - The BOLT (Betteer Optimization via Lookup-reuse and Two-pass) Interpreter uses a multi-pass optimization pipeline to accelerate IR evaluation.
     - [``wtk/bolt/Builder.h``](src/main/cpp/wtk/bolt/Builder.h) The first pass of `wtk::bolt` rebuilds the syntax-tree with pointers directly to wire storage.
     - [``wtk/bolt/Evaluator.h``](src/main/cpp/wtk/bolt/Evaluator.h) The second pass of `wtk::bolt` traverses the rebuilt tree, invoking the ZK Backend as necessary.
   - The PLASMASnooze (Practical Local Acceleration with Malleable Assumptions Snooze (opposite of FIREALARM)) interpreter is a single pass interpreter which relaxes conformace to the IR specification as much as is safe to improve performance (compared to FIREALARM).
     - [``wtk/bolt/PLASMASnooze.h``](src/main/cpp/wtk/bolt/PLASMASnooze.h) is the general-purpose PLASMASnooze interpreter.
     - [``wtk/bolt/ArithmeticPLASMASnoozeHandler.h``](src/main/cpp/wtk/bolt/ArithmeticPLASMASnoozeHandler.h) and [``wtk/bolt/BooleanPLASMASnoozeHandler.h``](src/main/cpp/wtk/bolt/BooleanPLASMASnoozeHandler.h``) are PLASMASnooze specializations for stream parsing and processing IR-Simple.

APIs and tools which we would like to have but haven't quite begun yet, and likely won't implement for IR v.1.x.x (e.g. wait for IR v2.0.0):

 - "TA1 Builder" This should be an API for building an IR syntax tree in-memory. Ideally where instance/short witness wires are needed, stubs could be stored to generate the correct value upon a traversal of the tree.
 - IR1 -> IR0 converter (API and tool). These are described in section 6 of the spec. Odds are we will implement only the ``@switch`` conversion for Phase I testing.
 - Naive Circuit Optimizer: just a tool for removing duplicate and unused gates.

## Credits
WizToolKit was developed by Stealth Software Technologies, Inc., and the primary and corresponding author is [Kimberlee Model](mailto:kimee@stealthsoftwareinc.com).
We would also like to acknowledge the efforts of Tarik Riviere and Xiao Wang in reviewing API designs and subsequently discovering bugs.
