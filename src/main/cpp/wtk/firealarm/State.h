/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_FIREALARM_STATE_H_
#define WTK_FIREALARM_STATE_H_

#include <cstddef>
#include <cstdint>

#include <unordered_map>

#include <wtk/index.h>
#include <wtk/Parser.h>

#include <wtk/firealarm/WireSet.h>

namespace wtk {
namespace firealarm {

/**
 * This is isomorphic to the "state" struct defined in 4.2 of the spec.
 *
 * Wire_T is the field-element type.
 */
template<typename Wire_T>
struct State
{
  /**
   * The wires in scope.
   */
  WireSet<Wire_T>* wires = nullptr;

  /**
   * The for-loop iterators in scope.
   */
  std::unordered_map<std::string, index_t>* iterators = nullptr;

  /**
   * The stream of instance values.
   */
  InputStream<Wire_T>* instance = nullptr;

  /**
   * The stream of short witness values.
   */
  InputStream<Wire_T>* shortWitness = nullptr;

  /**
   * Flag indicating if in an active switch case (normally true, false in
   * a switch inactive-case).
   */
  bool switchActive = true;

  State(WireSet<Wire_T>* w, std::unordered_map<std::string, index_t>* its,
      InputStream<Wire_T>* ins, InputStream<Wire_T>* wit)
    : wires(w), iterators(its), instance(ins), shortWitness(wit) { }

  /**
   * Indicates if FIREALARM should check for evaluation valididty.
   * The check is based on presence of instance and short_witness.
   */
  bool checkEvaluation()
  {
    return this->instance != nullptr && this->shortWitness != nullptr;
  }
};

} } // namespace wtk::firealarm

#endif//WTK_FIREALARM_STATE_H_
