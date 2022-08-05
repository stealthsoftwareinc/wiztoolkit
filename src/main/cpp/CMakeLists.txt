list(APPEND wiztoolkit_h
  wtk/ArithmeticStreamHandler.h
  wtk/ArithmeticStreamHandler.t.h
  wtk/BooleanStreamHandler.h
  wtk/IRParameters.h
  wtk/IRTree.h
  wtk/IRTree.t.h
  wtk/Parser.h
  wtk/Parser.t.h
  wtk/Version.h
  wtk/index.h
  wtk/QueueInputStream.h
  wtk/QueueInputStream.t.h
)

list(APPEND wiztoolkit_cpp
  wtk/BooleanStreamHandler.cpp
  wtk/IRTree.cpp
  wtk/IRParameters.cpp
)

LIST(APPEND irregular_h
  wtk/irregular/AutomataCtx.h
  wtk/irregular/checkMacros.t.h
  wtk/irregular/Parser.h
  wtk/irregular/Parser.t.h
  wtk/irregular/ParseEnums.h
  wtk/irregular/TextTreeParser.t.h
  wtk/irregular/TextIRTree.t.h
)

LIST(APPEND irregular_cpp
  wtk/irregular/AutomataCtx.cpp
)

if(${ENABLE_ANTLR} EQUAL 1)
  LIST(APPEND antlr_h
    wtk/antlr/Parser.h
    wtk/antlr/Parser.t.h
    wtk/antlr/to_uint.t.h
    wtk/antlr/AntlrIRTree.t.h
    wtk/antlr/GateUtils.t.h
    wtk/antlr/TreeParser.t.h
    wtk/antlr/ErrorListener.t.h
    ${gen_antlr_h}
  )

  LIST(APPEND antlr_cpp
  )

  set(ANTLR_LIBRARY antlr4_static)
  set(ANTLR_INCLUDE ../../deps/antlr4/runtime/src)
else()
  LIST(APPEND antlr_h)
  LIST(APPEND antlr_cpp)

  set(ANTLR_LIBRARY)
  set(ANTLR_INCLUDE)
endif()

if(${ENABLE_FLATBUFFER} EQUAL 1)
  LIST(APPEND flatbuffer_h
    wtk/flatbuffer/Parser.h
    wtk/flatbuffer/Parser.t.h
    wtk/flatbuffer/TreeParser.t.h
    wtk/flatbuffer/FBIRTree.t.h
    wtk/flatbuffer/FlatNumberHelper.t.h
    wtk/flatbuffer/FlatBufferPrinter.h
    wtk/flatbuffer/FlatBufferPrinter.t.h
    wtk/flatbuffer/ArithmeticFlatBufferStreamPrinter.h
    wtk/flatbuffer/ArithmeticFlatBufferStreamPrinter.t.h
    wtk/flatbuffer/BooleanFlatBufferStreamPrinter.h
    wtk/flatbuffer/FlatBufferTreePrinter.h
    wtk/flatbuffer/FlatBufferTreePrinter.t.h
  )

  LIST(APPEND flatbuffer_cpp
    wtk/flatbuffer/BooleanFlatBufferStreamPrinter.cpp
  )

  set(FLATBUFFER_INCLUDE ../../deps/flatbuffer/include)
else()
  LIST(APPEND flatbuffer_h)
  LIST(APPEND flatbuffer_cpp)

  set(FLATBUFFER_INCLUDE)
endif()

LIST(APPEND printers_h
  wtk/printers/printTextParameters.h
  wtk/printers/printTextParameters.t.h
  wtk/printers/ArithmeticTextStreamPrinter.h
  wtk/printers/ArithmeticTextStreamPrinter.t.h
  wtk/printers/BooleanTextStreamPrinter.h
  wtk/printers/TextTreePrinter.h
  wtk/printers/TextTreePrinter.t.h
)

LIST(APPEND printers_cpp
  wtk/printers/BooleanTextStreamPrinter.cpp
)

LIST(APPEND firealarm_h
  wtk/firealarm/WireSet.h
  wtk/firealarm/WireSet.t.h
  wtk/firealarm/State.h
  wtk/firealarm/TreeAlarm.h
  wtk/firealarm/TreeAlarm.t.h
  wtk/firealarm/TraceTreeAlarm.h
  wtk/firealarm/TraceTreeAlarm.t.h
)

LIST(APPEND firealarm_cpp
  wtk/firealarm/WireSet.cpp
)

LIST(APPEND firealarm_main
  wtk/firealarm/main.cpp
)

list(APPEND press_main
  wtk/press.cpp
)

LIST(APPEND utils_h
  wtk/utils/FileNameUtils.h
  wtk/utils/hints.h
  wtk/utils/is_prime.h
  wtk/utils/is_prime.t.h
  wtk/utils/NumUtils.h
  wtk/utils/NumUtils.t.h
  wtk/utils/Pool.h
  wtk/utils/Pool.t.h
  wtk/utils/SkipList.h
  wtk/utils/SkipList.t.h
  wtk/utils/Indent.h
  wtk/utils/IRTreeUtils.h
  wtk/utils/IRTreeUtils.t.h
)

LIST(APPEND utils_cpp
  wtk/utils/FileNameUtils.cpp
  wtk/utils/NumUtils.cpp
  wtk/utils/IRTreeUtils.cpp
)

LIST(APPEND viz_h
  wtk/viz/TreeVisualizer.h
  wtk/viz/TreeVisualizer.t.h
)

LIST(APPEND viz_cpp
)

LIST(APPEND viz_main
  wtk/viz/main.cpp
)

LIST(APPEND converters_h
  wtk/converters/Multiplex.h
  wtk/converters/Multiplex.t.h
  wtk/converters/TreeHelper.h
  wtk/converters/TreeHelper.t.h
)

LIST(APPEND converters_cpp
)

LIST(APPEND bolt_h
  wtk/bolt/wires.h
  wtk/bolt/wires.t.h
  wtk/bolt/directives.h
  wtk/bolt/directives.t.h
  wtk/bolt/Builder.h
  wtk/bolt/Builder.t.h
  wtk/bolt/Expr.h
  wtk/bolt/SwitchStreamHandler.h
  wtk/bolt/SwitchStreamHandler.t.h
  wtk/bolt/Evaluator.h
  wtk/bolt/Evaluator.t.h
  wtk/bolt/Backend.h
  wtk/bolt/Backend.t.h
  wtk/bolt/NoZKBackend.h
  wtk/bolt/PLASMASnooze.h
  wtk/bolt/PLASMASnooze.t.h
  wtk/bolt/ArithmeticPLASMASnoozeHandler.h
)

LIST(APPEND bolt_cpp
  wtk/bolt/Expr.cpp
)

LIST(APPEND bolt_main
  wtk/bolt/main.cpp
)

include_directories(
  .
  ../../../target/generated
  ../../deps/logging
  ${ANTLR_INCLUDE}
  ${FLATBUFFER_INCLUDE}
)

add_library(wiztoolkit
  ${irregular_h}
  ${irregular_cpp}
  ${gen_irregular_h}
  ${gen_irregular_cpp}
  ${antlr_h}
  ${antlr_cpp}
  ${gen_antlr_cpp}
  ${flatbuffer_h}
  ${flatbuffer_cpp}
  ${gen_flatbuffer_h}
  ${printers_h}
  ${printers_cpp}
  ${firealarm_h}
  ${firealarm_cpp}
  ${utils_h}
  ${utils_cpp}
  ${wiztoolkit_h}
  ${wiztoolkit_cpp}
  ${viz_h}
  ${viz_cpp}
  ${converters_h}
  ${converters_cpp}
  ${bolt_h}
  ${bolt_cpp}
)

target_link_libraries(wiztoolkit
  ${ANTLR_LIBRARY}
  stealth_logging
)

include_directories(
  ../../deps/sst_bignum/include
)

add_executable(wtk-firealarm
  ${firealarm_main}
)

target_link_libraries(wtk-firealarm
  sst_bignum
  ${OPENSSL_CRYPTO_LIBRARIES}
  wiztoolkit
  stealth_logging
  ${ANTLR_LIBRARY}
)

add_executable(wtk-press
  ${press_main}
)

target_link_libraries(wtk-press
  sst_bignum
  ${OPENSSL_CRYPTO_LIBRARIES}
  wiztoolkit
  stealth_logging
  ${ANTLR_LIBRARY}
)

add_executable(wtk-viz
  ${viz_main}
)

target_link_libraries(wtk-viz
  sst_bignum
  ${OPENSSL_CRYPTO_LIBRARIES}
  wiztoolkit
  stealth_logging
  ${ANTLR_LIBRARY}
)

add_executable(wtk-bolt
  ${bolt_main}
)

target_link_libraries(wtk-bolt
  sst_bignum
  ${OPENSSL_CRYPTO_LIBRARIES}
  wiztoolkit
  stealth_logging
  ${ANTLR_LIBRARY}
)

install(FILES
  ${wiztoolkit_h}
  DESTINATION include/wtk
)

install(FILES
  ${irregular_h}
  ${gen_irregular_h}
  DESTINATION include/wtk/irregular
)

install(FILES
  ${antlr_h}
  DESTINATION include/wtk/antlr
)

install(FILES
  ${flatbufer_h}
  DESTINATION include/wtk/flatbufffer
)

install(FILES
  ${printers_h}
  DESTINATION include/wtk/printers
)

install(FILES
  ${firealarm_h}
  DESTINATION include/wtk/firealarm
)

install(FILES
  ${utils_h}
  DESTINATION include/wtk/utils
)

install(FILES
  ${viz_h}
  DESTINATION include/wtk/viz
)

install(FILES
  ${converters_h}
  DESTINATION include/wtk/converters
)

install(FILES
  ${bolt_h}
  DESTINATION include/wtk/bolt
)

install(TARGETS wiztoolkit DESTINATION lib)
install(TARGETS wtk-firealarm DESTINATION bin)
install(TARGETS wtk-press DESTINATION bin)
install(TARGETS wtk-viz DESTINATION bin)
install(TARGETS wtk-bolt DESTINATION bin)

list(APPEND clang_tidy_sources
  ${irregular_cpp}
  ${gen_irregular_cpp}
  ${antlr_cpp}
  ${flatbuffer_cpp}
  ${printers_cpp}
  ${firealarm_cpp}
  ${utils_cpp}
  ${wiztoolkit_cpp}
  ${viz_cpp}
  ${converters_cpp}
  ${bolt_cpp}
  ${firealarm_main}
  ${press_main}
  ${viz_main}
  ${bolt_main}
)

add_custom_target(static-analysis
  clang-tidy -p ${CMAKE_BINARY_DIR}/compile_commands.json ${clang_tidy_sources}
  DEPENDS ${clang_tidy_sources}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/main/cpp)
