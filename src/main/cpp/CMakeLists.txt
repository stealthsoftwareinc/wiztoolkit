cmake_minimum_required(VERSION 3.1)

add_compile_options(${wtk_cxx_flags})

list(APPEND wiztoolkit_h
  wtk/Parser.h
  wtk/TypeBackend.h
  wtk/Converter.h
  wtk/indexes.h
  wtk/versions.h
)

list(APPEND wiztoolkit_cpp
)

list(APPEND utils_h
  wtk/utils/CharMap.h
  wtk/utils/hints.h
  wtk/utils/Indent.h
  wtk/utils/NumUtils.h
  wtk/utils/NumUtils.t.h
  wtk/utils/NumUtils.gmp.h
  wtk/utils/ParserOrganizer.h
  wtk/utils/ParserOrganizer.t.h
  wtk/utils/Pool.h
  wtk/utils/Pool.t.h
  wtk/utils/SkipList.h
  wtk/utils/SkipList.t.h
)

list(APPEND utils_cpp
  wtk/utils/CharMap.cpp
  wtk/utils/NumUtils.cpp
)

list(APPEND circuit_h
  wtk/circuit/Data.h
  wtk/circuit/Data.t.h
  wtk/circuit/Handler.h
  wtk/circuit/Parser.h
)

list(APPEND circuit_cpp
)

list(APPEND irregular_h
  wtk/irregular/AutomataCtx.h
  wtk/irregular/Parser.h
  wtk/irregular/Parser.t.h
  wtk/irregular/CircuitIR.i.h
)

list(APPEND irregular_cpp
  wtk/irregular/AutomataCtx.cpp
)

list(APPEND nails_h
  wtk/nails/Converter.h
  wtk/nails/Converter.t.h
  wtk/nails/Functions.h
  wtk/nails/Functions.t.h
  wtk/nails/Handler.h
  wtk/nails/Handler.t.h
  wtk/nails/Interpreter.h
  wtk/nails/Interpreter.t.h
  wtk/nails/IterPlugin.h
  wtk/nails/IterPlugin.t.h
  wtk/nails/Plugins.h
  wtk/nails/Plugins.t.h
  wtk/nails/Scope.h
  wtk/nails/Scope.t.h
  wtk/nails/TypeInterpreter.h
  wtk/nails/TypeInterpreter.t.h
)


list(APPEND nails_cpp
  wtk/nails/Scope.cpp
)

list(APPEND firealarm_h
  wtk/firealarm/Converter.h
  wtk/firealarm/Converter.t.h
  wtk/firealarm/Counters.h
  wtk/firealarm/RAM.h
  wtk/firealarm/RAM.t.h
  wtk/firealarm/FieldBackend.h
  wtk/firealarm/FieldBackend.t.h
  wtk/firealarm/RingBackend.h
  wtk/firealarm/RingBackend.t.h
  wtk/firealarm/Wire.h
  wtk/firealarm/Wire.t.h
)

list(APPEND firealarm_cpp
)

list(APPEND firealarm_main
  wtk/firealarm/main.cpp
)

list(APPEND plugins_h
  wtk/plugins/ArithmeticRAM.h
  wtk/plugins/ArithmeticRAM.t.h
  wtk/plugins/BooleanRAM.h
  wtk/plugins/BooleanRAM.t.h
  wtk/plugins/Multiplexer.h
  wtk/plugins/Multiplexer.t.h
  wtk/plugins/PluginBackend.h
  wtk/plugins/PluginBackend.t.h
  wtk/plugins/Plugin.h
  wtk/plugins/Plugin.t.h
  wtk/plugins/SimplePlugin.h
  wtk/plugins/SimplePlugin.t.h
  wtk/plugins/Vectors.h
  wtk/plugins/Vectors.t.h
  wtk/plugins/ExtendedArithmetic.h
  wtk/plugins/ExtendedArithmetic.t.h
)

list(APPEND plugins_cpp
)

list(APPEND press_h
  wtk/press/Printer.h
  wtk/press/TextPrinter.h
  wtk/press/FlatbufferPrinter.h
  wtk/press/NothingPrinter.h
)

list(APPEND press_cpp
)

list(APPEND press_main
  wtk/press/main.cpp
)

if(${ENABLE_FLATBUFFER} EQUAL 1)
  LIST(APPEND flatbuffer_h
    wtk/flatbuffer/Parser.h
    wtk/flatbuffer/Parser.t.h
  )

  LIST(APPEND flatbuffer_cpp
  )

  set(FLATBUFFER_INCLUDE ../../deps/flatbuffer/include)
else()
  LIST(APPEND flatbuffer_h)
  LIST(APPEND flatbuffer_cpp)

  set(FLATBUFFER_INCLUDE)
endif()

include_directories(
  .
  ../../../target/generated
  ../../deps/logging
  ../../deps/sst_bignum/include
  ${FLATBUFFER_INCLUDE}
)

add_library(wiztoolkit
  ${wiztoolkit_h}
  ${wiztoolkit_cpp}
  ${utils_h}
  ${utils_cpp}
  ${circuit_h}
  ${circuit_cpp}
  ${irregular_h}
  ${irregular_cpp}
  ${gen_irregular_h}
  ${gen_irregular_cpp}
  ${nails_h}
  ${nails_cpp}
  ${firealarm_h}
  ${firealarm_cpp}
  ${plugins_h}
  ${plugins_cpp}
  ${flatbuffer_h}
  ${flatbuffer_cpp}
  ${gen_flatbuffer_h}
  ${press_h}
  ${press_cpp}
)

target_compile_features(wiztoolkit PUBLIC cxx_std_11)

target_link_libraries(wiztoolkit
  PUBLIC stealth_logging
  PRIVATE
)

add_executable(wtk-firealarm
  ${firealarm_main}
)

target_link_libraries(wtk-firealarm PRIVATE
  sst_bignum
  ${OPENSSL_CRYPTO_LIBRARIES}
  wiztoolkit
  stealth_logging
)

add_executable(wtk-press
  ${press_main}
)

target_link_libraries(wtk-press PRIVATE
  sst_bignum
  ${OPENSSL_CRYPTO_LIBRARIES}
  wiztoolkit
  stealth_logging
)

install(TARGETS wiztoolkit DESTINATION lib)
install(TARGETS wtk-firealarm DESTINATION bin)
install(TARGETS wtk-press DESTINATION bin)

install(FILES ${wiztoolkit_h} DESTINATION include/wtk)
install(FILES ${utils_h} DESTINATION include/wtk/utils)
install(FILES ${circuit_h} DESTINATION include/wtk/circuit)
install(FILES
  ${irregular_h}
  ${gen_irregular_h}
  DESTINATION include/wtk/irregular
)
install(FILES ${nails_h} DESTINATION include/wtk/nails)
install(FILES ${firealarm_h} DESTINATION include/wtk/firealarm)
install(FILES ${plugins_h} DESTINATION include/wtk/plugins)
install(FILES
  ${flatbuffer_h}
  ${gen_flatbuffer_h}
  DESTINATION include/wtk/flatbuffer
)
install(FILES ${press_h} DESTINATION include/wtk/press)

list(APPEND clang_tidy_sources
  ${wiztoolkit_cpp}
  ${utils_cpp}
  ${circuit_cpp}
  ${irregular_cpp}
  ${gen_irregular_cpp}
  ${nails_cpp}
  ${firealarm_cpp}
  ${firealarm_main}
  ${flatbuffer_cpp}
)

add_custom_target(static-analysis
  clang-tidy -p ${CMAKE_BINARY_DIR}/compile_commands.json ${clang_tidy_sources}
  DEPENDS ${clang_tidy_sources}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/main/cpp)
