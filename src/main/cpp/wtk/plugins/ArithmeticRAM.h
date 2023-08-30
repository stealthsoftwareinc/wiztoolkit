/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

#ifndef WTK_PLUGINS_ARITHMETIC_RAM_PLUGIN_H_
#define WTK_PLUGINS_ARITHMETIC_RAM_PLUGIN_H_

#include <cstddef>
#include <cstdlib>
#include <vector>

#include <wtk/indexes.h>
#include <wtk/circuit/Data.h>
#include <wtk/plugins/PluginBackend.h>
#include <wtk/plugins/SimplePlugin.h>
#include <wtk/plugins/Multiplexer.h>
#include <wtk/utils/NumUtils.h>

namespace wtk {
namespace plugins {

/**
 * A backend interface for the RAM buffer type. On the IR side, it does next
 * to nothing, because all interactions occur through the plugin-functions.
 * But it's a good spot to wrap up pointers to the element/index wire.
 *
 * The Number_T is of course the numeric type for plain numbers.
 * The Buffer_T takes the place of Wire_T, because the buffer is a wire
 * who's value happens to be a RAM buffer.
 * The Wire_T name is reused for the index/element type.
 */
template<typename Number_T, typename Buffer_T, typename Wire_T>
struct RAMBackend : public PluginBackend<Number_T, Buffer_T>
{
  /**
   * Index of the index/element's type.
   */
  wtk::type_idx const wireType;

  /**
   * The backend for the index/element's backend.
   */
  wtk::TypeBackend<Number_T, Wire_T>* const wireBackend;

  RAMBackend(wtk::circuit::TypeSpec<Number_T> const* const tp,
      wtk::type_idx const wt, wtk::TypeBackend<Number_T, Wire_T>* const wb)
    : PluginBackend<Number_T, Buffer_T>(tp), wireType(wt), wireBackend(wb) { }
};

/**
 * An operation to initialize RAM buffers.
 */
template<typename Number_T, typename Buffer_T, typename Wire_T>
struct RAMInitOperation : public SimpleOperation<Number_T, Buffer_T>
{
  RAMInitOperation(wtk::type_idx const t,
      RAMBackend<Number_T, Buffer_T, Wire_T>* const be)
    : SimpleOperation<Number_T, Buffer_T>(t, be) { }

  void evaluate(
      std::vector<WiresRefEraser>& outputs,
      std::vector<WiresRefEraser>& inputs,
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) final;

  bool checkSignature(
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) final;

  /**
   * Helper for casting the backend to a RAM Backend.
   */
  RAMBackend<Number_T, Buffer_T, Wire_T>* ramBackend()
  {
    return static_cast<RAMBackend<Number_T, Buffer_T, Wire_T>*>(this->backend);
  }

  /**
   * Implement this function to initialize a RAM buffer. Assign every element
   * in the buffer the initial value of fill.
   *
   * size is the length of the buffer.
   * buffer is a default constructed RAM Buffer, to be properly intialized.
   * fill is the value to give each initial RAM value.
   */
  virtual void init(wtk::wire_idx const size,
      Buffer_T* const buffer, Wire_T const* const fill) = 0;
};

/**
 * An operation to read values from RAM buffers.
 */
template<typename Number_T, typename Buffer_T, typename Wire_T>
struct RAMReadOperation : public SimpleOperation<Number_T, Buffer_T>
{
  RAMReadOperation(wtk::type_idx const t,
      RAMBackend<Number_T, Buffer_T, Wire_T>* const be)
    : SimpleOperation<Number_T, Buffer_T>(t, be) { }

  void evaluate(
      std::vector<WiresRefEraser>& outputs,
      std::vector<WiresRefEraser>& inputs,
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) final;

  bool checkSignature(
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) final;

  /**
   * Helper for casting the backend to a RAM Backend.
   */
  RAMBackend<Number_T, Buffer_T, Wire_T>* ramBackend()
  {
    return static_cast<RAMBackend<Number_T, Buffer_T, Wire_T>*>(this->backend);
  }

  /**
   * Implement this function to read the specified element from buffer.
   *
   *   out = buffer[idx]
   */
  virtual void read(Wire_T* const out,
      Buffer_T* const buffer, Wire_T const* const idx) = 0;
};

/**
 * An operation to write values into RAM buffers.
 */
template<typename Number_T, typename Buffer_T, typename Wire_T>
struct RAMWriteOperation : public SimpleOperation<Number_T, Buffer_T>
{
  RAMWriteOperation(wtk::type_idx const t,
      RAMBackend<Number_T, Buffer_T, Wire_T>* const be)
    : SimpleOperation<Number_T, Buffer_T>(t, be) { }

  void evaluate(
      std::vector<WiresRefEraser>& outputs,
      std::vector<WiresRefEraser>& inputs,
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) final;

  bool checkSignature(
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) final;

  /**
   * Helper for casting the backend to a RAM Backend.
   */
  RAMBackend<Number_T, Buffer_T, Wire_T>* ramBackend()
  {
    return static_cast<RAMBackend<Number_T, Buffer_T, Wire_T>*>(this->backend);
  }

  /**
   * Implement this function to write a value to the specified buffer element.
   *
   *   buffer[idx] = in;
   */
  virtual void write(Buffer_T* const buffer,
      Wire_T const* const idx, Wire_T const* const in) = 0;
};

template<typename Number_T, typename Buffer_T, typename Wire_T>
struct RAMPlugin : public SimplePlugin<Number_T, Buffer_T>
{
  bool buildBackend(wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Buffer_T>* const ram_backend,
      wtk::utils::CharMap<std::unique_ptr<
        wtk::plugins::SimpleOperation<
          Number_T, Buffer_T>>>* const operations) final;

  virtual RAMInitOperation<Number_T, Buffer_T, Wire_T>* buildInitOperation(
      wtk::type_idx const type,
      RAMBackend<Number_T, Buffer_T, Wire_T>* const be) = 0;

  virtual RAMReadOperation<Number_T, Buffer_T, Wire_T>* buildReadOperation(
      wtk::type_idx const type,
      RAMBackend<Number_T, Buffer_T, Wire_T>* const be) = 0;

  virtual RAMWriteOperation<Number_T, Buffer_T, Wire_T>* buildWriteOperation(
      wtk::type_idx const type,
      RAMBackend<Number_T, Buffer_T, Wire_T>* const be) = 0;
};

/**
 * A helper function for checking the RAM plugin type when using the v0.
 *
 * The ram type has the form
 *
 *   @plugin(ram_arith_v0, ram,
 *       <idx-type>, <num-allocs>, <total-alloc>, <max-alloc>)
 *
 *  - idx-type:    the type index for the buffer's index/element type.
 *  - num-allocs:  the number of allocations made by the circuit.
 *  - total-alloc: the sum/total size of all allocations.
 *  - max-alloc:   the maximum total size of concurrently allocated buffers.
 */
template<typename Number_T>
bool checkRAMv0Type(wtk::circuit::TypeSpec<Number_T> const* const type,
    wtk::type_idx* const idx_type, wtk::wire_idx* const num_allocs,
    wtk::wire_idx* const total_allocs, wtk::wire_idx* const max_alloc);

/** ==== Fallback implementation of RAM ==== */

template<typename Wire_T>
struct FallbackRAMBuffer
{
  size_t size = 0;

  Wire_T* buffer = nullptr;

  FallbackRAMBuffer() = default;

  FallbackRAMBuffer(FallbackRAMBuffer const& copy) = delete;
  FallbackRAMBuffer& operator=(FallbackRAMBuffer const& copy) = delete;

  FallbackRAMBuffer(FallbackRAMBuffer&& move) = default;
  FallbackRAMBuffer& operator=(FallbackRAMBuffer&& move) = default;

  ~FallbackRAMBuffer();
};

template<typename Number_T, typename Wire_T>
struct FallbackRAMBackend
 : public RAMBackend<Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>
{
  FallbackRAMBackend(wtk::circuit::TypeSpec<Number_T> const* const tp,
      wtk::type_idx const wt, wtk::TypeBackend<Number_T, Wire_T>* const wb)
    : RAMBackend<Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>(tp, wt, wb) { }

  bool check() override;
};

template<typename Number_T, typename Wire_T>
struct FallbackRAMInitOperation
  : public RAMInitOperation<Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>
{
  FallbackRAMInitOperation(wtk::type_idx const t,
      RAMBackend<Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>* const be)
    : RAMInitOperation<Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>(t, be) { }

  void init(wtk::wire_idx const size,
      FallbackRAMBuffer<Wire_T>* const buffer,
      Wire_T const* const fill) override;
};

template<typename Number_T, typename Wire_T>
struct FLTRAMReadOperation
  : public RAMReadOperation<Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>
{
  FLTRAMReadOperation(wtk::type_idx const t,
      RAMBackend<Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>* const be)
    : RAMReadOperation<Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>(t, be) { }

  void read(Wire_T* const out,
      FallbackRAMBuffer<Wire_T>* const buffer,
      Wire_T const* const idx) override;
};

template<typename Number_T, typename Wire_T>
struct FLTRAMWriteOperation
  : public RAMWriteOperation<Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>
{
  FLTRAMWriteOperation(wtk::type_idx const t,
      RAMBackend<Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>* const be)
    : RAMWriteOperation<Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>(t, be) { }

  void write(FallbackRAMBuffer<Wire_T>* const buffer,
      Wire_T const* const idx, Wire_T const* const in) override;
};

template<typename Number_T, typename Wire_T>
struct FallbackRAMPlugin
  : public RAMPlugin<Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>
{

  RAMInitOperation<
    Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>* buildInitOperation(
        wtk::type_idx const type, RAMBackend<
          Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>* const be) override;

  RAMReadOperation<
    Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>* buildReadOperation(
        wtk::type_idx const type, RAMBackend<
          Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>* const be) override;

  RAMWriteOperation<
    Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>* buildWriteOperation(
        wtk::type_idx const type, RAMBackend<
          Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>* const be) override;
};

} } // namespace wtk::plugins

#define LOG_IDENTIFIER "ram_arith_v0"
#include <stealth_logging.h>

#include <wtk/plugins/ArithmeticRAM.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_PLUGINS_ARITHMETIC_RAM_PLUGIN_H_
