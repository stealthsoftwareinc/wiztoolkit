/**
 * Copyright (C) 2022 - 2023, Stealth Software Technologies, Inc.
 */

#ifndef WTK_PLUGINS_VECTORS_H_
#define WTK_PLUGINS_VECTORS_H_

#include <cstddef>
#include <memory>

#include <wtk/indexes.h>
#include <wtk/TypeBackend.h>
#include <wtk/circuit/Data.h>
#include <wtk/utils/CharMap.h>

#include <wtk/plugins/SimplePlugin.h>

namespace wtk {
namespace plugins {

template<typename Number_T, typename Wire_T>
struct PairwiseOperation : public SimpleOperation<Number_T, Wire_T>
{
  PairwiseOperation(type_idx const type,
      TypeBackend<Number_T, Wire_T>* const backend)
    : SimpleOperation<Number_T, Wire_T>(type, backend) { }

  bool checkSignature(
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) final;

  void evaluate(std::vector<WiresRefEraser>& outputs,
      std::vector<WiresRefEraser>& inputs) final;

  virtual void evaluatePairwise(size_t const size, Wire_T* const outs,
      Wire_T const* const lefts,  Wire_T const* const rights) = 0;
};

template<typename Number_T, typename Wire_T>
struct UniFoldOperation : public SimpleOperation<Number_T, Wire_T>
{
  UniFoldOperation(type_idx const type,
      TypeBackend<Number_T, Wire_T>* const backend)
    : SimpleOperation<Number_T, Wire_T>(type, backend) { }

  bool checkSignature(
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) final;

  void evaluate(std::vector<WiresRefEraser>& outputs,
      std::vector<WiresRefEraser>& inputs) final;

  virtual void evaluateUniFold(
      size_t const size, Wire_T* const out, Wire_T const* const ins) = 0;
};

template<typename Number_T, typename Wire_T>
struct BiFoldOperation : public SimpleOperation<Number_T, Wire_T>
{
  BiFoldOperation(type_idx const type,
      TypeBackend<Number_T, Wire_T>* const backend)
    : SimpleOperation<Number_T, Wire_T>(type, backend) { }

  bool checkSignature(
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) final;

  void evaluate(std::vector<WiresRefEraser>& outputs,
      std::vector<WiresRefEraser>& inputs) final;

  virtual void evaluateBiFold(size_t const size, Wire_T* const out,
      Wire_T const* const lefts, Wire_T const* const rights) = 0;
};

template<typename Number_T, typename Wire_T>
struct VectorPlugin : public SimplePlugin<Number_T, Wire_T>
{
  bool buildBackend(wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend,
      wtk::utils::CharMap<std::unique_ptr<
        SimpleOperation<Number_T, Wire_T>>>* const operations) final;

  /**
   * The following should return new XOperation<...> as needed. The returned
   * pointers will be deleted by the VectorPlugin
   */
  virtual PairwiseOperation<Number_T, Wire_T>* buildAdd(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) = 0;

  virtual PairwiseOperation<Number_T, Wire_T>* buildMul(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) = 0;

  virtual UniFoldOperation<Number_T, Wire_T>* buildSum(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) = 0;

  virtual UniFoldOperation<Number_T, Wire_T>* buildProduct(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) = 0;

  virtual BiFoldOperation<Number_T, Wire_T>* buildDotProduct(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) = 0;
};

// ==== Fallback operations and plugin ====

template<typename Number_T, typename Wire_T>
struct FallbackAddOperation : public PairwiseOperation<Number_T, Wire_T>
{
  FallbackAddOperation(wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend)
    : PairwiseOperation<Number_T, Wire_T>(type, backend) { }

  void evaluatePairwise(size_t const size, Wire_T* const outs,
      Wire_T const* const lefts,  Wire_T const* const rights) final;
};

template<typename Number_T, typename Wire_T>
struct FallbackMulOperation : public PairwiseOperation<Number_T, Wire_T>
{
  FallbackMulOperation(wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend)
    : PairwiseOperation<Number_T, Wire_T>(type, backend) { }

  void evaluatePairwise(size_t const size, Wire_T* const outs,
      Wire_T const* const lefts,  Wire_T const* const rights) final;
};

template<typename Number_T, typename Wire_T>
struct FallbackSumOperation : public UniFoldOperation<Number_T, Wire_T>
{
  FallbackSumOperation(wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend)
    : UniFoldOperation<Number_T, Wire_T>(type, backend) { }

  void evaluateUniFold(
      size_t const size, Wire_T* const out, Wire_T const* const ins) final;
};

template<typename Number_T, typename Wire_T>
struct FallbackProductOperation : public UniFoldOperation<Number_T, Wire_T>
{
  FallbackProductOperation(wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend)
    : UniFoldOperation<Number_T, Wire_T>(type, backend) { }

  void evaluateUniFold(
      size_t const size, Wire_T* const out, Wire_T const* const ins) final;
};

template<typename Number_T, typename Wire_T>
struct FallbackDotProductOperation : public BiFoldOperation<Number_T, Wire_T>
{
  FallbackDotProductOperation(wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend)
    : BiFoldOperation<Number_T, Wire_T>(type, backend) { }

  void evaluateBiFold(size_t const size, Wire_T* const out,
      Wire_T const* const lefts, Wire_T const* const rights) final;
};

template<typename Number_T, typename Wire_T>
struct FallbackVectorPlugin : public VectorPlugin<Number_T, Wire_T>
{
  PairwiseOperation<Number_T, Wire_T>* buildAdd(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) override;

  PairwiseOperation<Number_T, Wire_T>* buildMul(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) override;

  UniFoldOperation<Number_T, Wire_T>* buildSum(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) override;

  UniFoldOperation<Number_T, Wire_T>* buildProduct(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) override;

  BiFoldOperation<Number_T, Wire_T>* buildDotProduct(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) override;
};

} } // namespace wtk::plugins

#define LOG_IDENTIFIER "wizkit_vectors"
#include <stealth_logging.h>

#include <wtk/plugins/Vectors.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_PLUGINS_VECTORS_H_
