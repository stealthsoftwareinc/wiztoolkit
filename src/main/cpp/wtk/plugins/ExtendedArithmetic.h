/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

#ifndef WTK_PLUGINS_EXTENDED_ARITHMETIC_H_
#define WTK_PLUGINS_EXTENDED_ARITHMETIC_H_

#include <cstddef>
#include <vector>

#include <wtk/indexes.h>
#include <wtk/TypeBackend.h>
#include <wtk/utils/CharMap.h>
#include <wtk/circuit/Data.h>
#include <wtk/utils/hints.h>
#include <wtk/utils/ParserOrganizer.h>

#include <wtk/plugins/SimplePlugin.h>

namespace wtk {
namespace plugins {

/**
 * An operation super class to perform comparisons in arithmetic fields.
 */
template<typename Number_T, typename Wire_T>
struct ComparisonOperation : public SimpleOperation<Number_T, Wire_T>
{
  ComparisonOperation(type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend)
    : SimpleOperation<Number_T, Wire_T>(type, backend) { }

  bool checkSignature(
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) final;

  void evaluate(std::vector<WiresRefEraser>& outputs,
      std::vector<WiresRefEraser>& inputs) final;

  /**
   * Virtual function for performing the comparison.
   *
   * out: evaluateCmp must assign 1 if the comparison is true, 0 otherwise
   * left, right: the operands to compare.
   */
  virtual void evaluateCmp(Wire_T* const out,
      Wire_T const* const left, Wire_T const* const right) = 0;
};

/**
 * An operation to perform division/remainder operations in arithmetic fields.
 */
template<typename Number_T, typename Wire_T>
struct DivisionOperation : public SimpleOperation<Number_T, Wire_T>
{
  DivisionOperation(type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend)
    : SimpleOperation<Number_T, Wire_T>(type, backend) { }

  bool checkSignature(
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) final;

  void evaluate(std::vector<WiresRefEraser>& outputs,
      std::vector<WiresRefEraser>& inputs) final;

  /**
   * Virtual function for performing integer-like division with remainder.
   *
   * quotient: The quotient of the division operation.
   * remainder: The remainder of the division operation.
   * left, right: the operands to divide (left // right).
   */
  virtual void evaluateDiv(Wire_T* const quotient, Wire_T* const remainder,
      Wire_T const* const left, Wire_T const* const right) = 0;
};

/**
 * An operation to perform bit decomposition operations in arithmetic fields.
 *
 * The bit-decomposition will use big-end first representation
 */
template<typename Number_T, typename Wire_T>
struct BitDecomposeOperation : public SimpleOperation<Number_T, Wire_T>
{
  // The number of bits needed to represent the prime.
  size_t bits;

  BitDecomposeOperation(type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend, size_t const bits)
    : SimpleOperation<Number_T, Wire_T>(type, backend), bits(bits) { }

  bool checkSignature(
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) final;

  void evaluate(std::vector<WiresRefEraser>& outputs,
      std::vector<WiresRefEraser>& inputs) final;

  /**
   * Virtual function for performing bit decomposition.
   *
   * out_bits: a pointer to a list of this->bits many output wires.
   * input: the input wire which will be decomposed
   */
  virtual void evaluateDecomp(
      Wire_T* const out_bits, Wire_T const* const input) = 0;
};

template<typename Number_T, typename Wire_T>
struct ExtendedArithmeticPlugin : public SimplePlugin<Number_T, Wire_T>
{
  bool buildBackend(wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend,
      wtk::utils::CharMap<std::unique_ptr<
        SimpleOperation<Number_T, Wire_T>>>* const operations) final;

  virtual ComparisonOperation<Number_T, Wire_T>* buildLessThan(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) = 0;

  virtual ComparisonOperation<Number_T, Wire_T>* buildLessThanEqual(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) = 0;

  virtual DivisionOperation<Number_T, Wire_T>* buildDivision(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) = 0;

  virtual BitDecomposeOperation<Number_T, Wire_T>* buildBitDecompose(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) = 0;
};

template<typename Number_T, typename Wire_T>
struct FallbackLessThanOperation : public ComparisonOperation<Number_T, Wire_T>
{
  std::vector<Number_T> primeDecomp;
  wtk::utils::Setting const setting;

  FallbackLessThanOperation(
      wtk::type_idx const t, wtk::TypeBackend<Number_T, Wire_T>* const be,
      std::vector<Number_T>&& pd, wtk::utils::Setting s);

  void evaluateCmp(Wire_T* const out,
      Wire_T const* const left, Wire_T const* const right) override;
};

template<typename Number_T, typename Wire_T>
struct FallbackLessThanEqualOperation
  : public ComparisonOperation<Number_T, Wire_T>
{
  std::vector<Number_T> primeDecomp;
  wtk::utils::Setting const setting;

  FallbackLessThanEqualOperation(
      wtk::type_idx const t, wtk::TypeBackend<Number_T, Wire_T>* const be,
      std::vector<Number_T>&& pd, wtk::utils::Setting s);

  void evaluateCmp(Wire_T* const out,
      Wire_T const* const left, Wire_T const* const right) override;
};

template<typename Number_T, typename Wire_T>
struct FallbackDivisionOperation : public DivisionOperation<Number_T, Wire_T>
{
  std::vector<Number_T> primeDecomp;
  wtk::utils::Setting const setting;

  FallbackDivisionOperation(
      wtk::type_idx const t, wtk::TypeBackend<Number_T, Wire_T>* const be,
      std::vector<Number_T>&& pd, wtk::utils::Setting s);

  void evaluateDiv(Wire_T* const quotient, Wire_T* const remainder,
      Wire_T const* const left, Wire_T const* const right) override;
};

template<typename Number_T, typename Wire_T>
struct FallbackBitDecomposeOperation
  : public BitDecomposeOperation<Number_T, Wire_T>
{
  std::vector<Number_T> primeDecomp;
  wtk::utils::Setting const setting;

  FallbackBitDecomposeOperation(
      wtk::type_idx const t, wtk::TypeBackend<Number_T, Wire_T>* const be,
      size_t b, std::vector<Number_T>&& pd, wtk::utils::Setting s);

  void evaluateDecomp(
      Wire_T* const out_bits, Wire_T const* const input) override;
};

template<typename Number_T, typename Wire_T>
struct FallbackExtendedArithmeticPlugin
  : public ExtendedArithmeticPlugin<Number_T, Wire_T>
{
  wtk::utils::Setting const setting;

  FallbackExtendedArithmeticPlugin(wtk::utils::Setting s) : setting(s) { }

  ComparisonOperation<Number_T, Wire_T>* buildLessThan(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) override;

  ComparisonOperation<Number_T, Wire_T>* buildLessThanEqual(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) override;

  DivisionOperation<Number_T, Wire_T>* buildDivision(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) override;

  BitDecomposeOperation<Number_T, Wire_T>* buildBitDecompose(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) override;
};

} } // namespace wtk::plugins

#include <wtk/utils/NumUtils.h>

#define LOG_IDENTIFIER "extended_arithmetic"
#include <stealth_logging.h>

#include <wtk/plugins/ExtendedArithmetic.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_PLUGINS_EXTENDED_ARITHMETIC_H_
