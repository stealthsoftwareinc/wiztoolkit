/**
 * Copyright (C) 2022 Stealth Software Technologies, Inc.
 */

#ifndef WTK_BOLT_EVALUATOR_H_
#define WTK_BOLT_EVALUATOR_H_

#include <vector>

#include <wtk/Parser.h>
#include <wtk/index.h>

#include <wtk/bolt/directives.h>
#include <wtk/bolt/SwitchStreamHandler.h>
#include <wtk/bolt/Backend.h>
#include <wtk/bolt/PLASMASnooze.h>

namespace wtk {
namespace bolt {

/**
 * The BOLT Evaluator encapsulates the second phase of BOLT invocation,
 * traversing the BOLT relation and "executing" each gate. This by definition
 * has to be O(n) where n is the total number of gates.
 */
template<typename Wire_T, typename Number_T>
class Evaluator
{
  Backend<Wire_T, Number_T>* const backend;

  // A stack for use while evaluating expressions.
  std::vector<wtk::index_t> exprStack;

public:

  /**
   * Constructor needs only a pointer to the backend.
   */
  Evaluator(Backend<Wire_T, Number_T>* const b) : backend(b) { }

  /**
   * Evaluate a BOLT Relation with a given instance and witness.
   * The BOLT relation is guaranteed well-formed by the BOLT Builder,
   * but this may fail if the instance or witness is poorly-formed.
   *
   * If the witness is nullptr, the Evaluator assumes that the backend is a
   * verifier and feeds zeroes in place of the witness.
   *
   * Proof validity is given by Backend.check().
   */
  bool evaluate(Bolt<Wire_T, Number_T>* const bolt,
      wtk::InputStream<Number_T>* const instance,
      wtk::InputStream<Number_T>* const witness);

private:
  // A helper function for the prior
  bool evaluate(Bolt<Wire_T, Number_T>* const bolt,
      wtk::InputStream<Number_T>* const instance,
      SwitchStreamHandler<Wire_T>* const switch_instance,
      wtk::InputStream<Number_T>* const witness,
      SwitchStreamHandler<Wire_T>* const switch_witness,
      Wire_T const* const enabled_bit);
};

} } // namespace wtk::bolt

#define LOG_IDENTIFIER "wtk::bolt"
#include <stealth_logging.h>

#include <wtk/bolt/Evaluator.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_BOLT_EVALUATOR_H_
