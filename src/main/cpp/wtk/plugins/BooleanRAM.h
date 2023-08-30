/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

#ifndef WTK_PLUGINS_BOOLEAN_RAM_PLUGIN_H_
#define WTK_PLUGINS_BOOLEAN_RAM_PLUGIN_H_

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
 * A TypeBackend interface for the Boolean RAM buffer type. It is largely
 * unused/no-op in terms of gate callbacks, but it is useful for holding on
 * to the index/element's wire types, along with bit-widths and other relevant
 * information.
 */
template<typename Number_T, typename Buffer_T, typename Wire_T>
struct BoolRAMBackend : public PluginBackend<Number_T, Buffer_T>
{
  /**
   * Index of the index/element's type.
   */
  wtk::type_idx const wireType;

  /**
   * The backend for the index/element's backend.
   */
  wtk::TypeBackend<Number_T, Wire_T>* const wireBackend;

  /**
   * Bit-width for the RAM's index.
   */
  wtk::wire_idx const indexBits;
  
  /**
   * Bit-width for the RAM's element.
   */
  wtk::wire_idx const elementBits;

  BoolRAMBackend(wtk::circuit::TypeSpec<Number_T> const* const tp,
      wtk::type_idx const wt, wtk::TypeBackend<Number_T, Wire_T>* const wb,
      wtk::wire_idx const ib, wtk::wire_idx const eb)
    : PluginBackend<Number_T, Buffer_T>(tp), wireType(wt), wireBackend(wb),
      indexBits(ib), elementBits(eb) { }
};

/**
 * An operation to initialize Boolean RAM buffers.
 */
template<typename Number_T, typename Buffer_T, typename Wire_T>
struct BoolRAMInitOperation : public SimpleOperation<Number_T, Buffer_T>
{
  BoolRAMInitOperation(wtk::type_idx const t,
      BoolRAMBackend<Number_T, Buffer_T, Wire_T>* const be)
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
   * Helper for casting the backend to a Boolean RAM Backend.
   */
  BoolRAMBackend<Number_T, Buffer_T, Wire_T>* ramBackend()
  {
    return static_cast<BoolRAMBackend<
      Number_T, Buffer_T, Wire_T>*>(this->backend);
  }

  /**
   * Implement this function to initialize a Boolean RAM buffer. Assign
   * every element in the buffer the initial value of fill.
   *
   * size is the number of elements in the buffer.
   * buffer is a default constructed RAM Buffer, to be properly intialized.
   * fill is the value to give each initial RAM value.
   *
   * The element bit-width is accessible via this->ramBackend()->elementBits.
   */
  virtual void init(wtk::wire_idx const size,
      Buffer_T* const buffer, Wire_T const* const fill) = 0;
};

/**
 * An operation to read values from Boolean RAM buffers.
 */
template<typename Number_T, typename Buffer_T, typename Wire_T>
struct BoolRAMReadOperation : public SimpleOperation<Number_T, Buffer_T>
{
  BoolRAMReadOperation(wtk::type_idx const t,
      BoolRAMBackend<Number_T, Buffer_T, Wire_T>* const be)
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
   * Helper for casting the backend to a Boolean RAM Backend.
   */
  BoolRAMBackend<Number_T, Buffer_T, Wire_T>* ramBackend()
  {
    return static_cast<BoolRAMBackend<
      Number_T, Buffer_T, Wire_T>*>(this->backend);
  }

  /**
   * Implement this function to read the specified element from buffer.
   *
   *   out = buffer[idx]
   *
   * Remember out and idx have bit-widths of this-ramBackend()->elementBits
   * and ->indexBits.
   */
  virtual void read(Wire_T* const out,
      Buffer_T* const buffer, Wire_T const* const idx) = 0;
};

/**
 * An operation to write values into Boolean RAM buffers.
 */
template<typename Number_T, typename Buffer_T, typename Wire_T>
struct BoolRAMWriteOperation : public SimpleOperation<Number_T, Buffer_T>
{
  BoolRAMWriteOperation(wtk::type_idx const t,
      BoolRAMBackend<Number_T, Buffer_T, Wire_T>* const be)
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
   * Helper for casting the backend to a Boolean RAM Backend.
   */
  BoolRAMBackend<Number_T, Buffer_T, Wire_T>* ramBackend()
  {
    return static_cast<BoolRAMBackend<
      Number_T, Buffer_T, Wire_T>*>(this->backend);
  }

  /**
   * Implement this function to write a value to the specified buffer element.
   *
   *   buffer[idx] = in;
   *
   * Remember in and idx have bit-widths of this-ramBackend()->elementBits
   * and ->indexBits.
   */
  virtual void write(Buffer_T* const buffer,
      Wire_T const* const idx, Wire_T const* const in) = 0;
};

template<typename Number_T, typename Buffer_T, typename Wire_T>
struct BoolRAMPlugin : public SimplePlugin<Number_T, Buffer_T>
{
  bool buildBackend(wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Buffer_T>* const ram_backend,
      wtk::utils::CharMap<std::unique_ptr<
        wtk::plugins::SimpleOperation<
          Number_T, Buffer_T>>>* const operations) final;

  virtual BoolRAMInitOperation<Number_T, Buffer_T, Wire_T>* buildInitOperation(
      wtk::type_idx const type,
      BoolRAMBackend<Number_T, Buffer_T, Wire_T>* const be) = 0;

  virtual BoolRAMReadOperation<Number_T, Buffer_T, Wire_T>* buildReadOperation(
      wtk::type_idx const type,
      BoolRAMBackend<Number_T, Buffer_T, Wire_T>* const be) = 0;

  virtual BoolRAMWriteOperation<Number_T, Buffer_T, Wire_T>*
    buildWriteOperation(
        wtk::type_idx const type,
        BoolRAMBackend<Number_T, Buffer_T, Wire_T>* const be) = 0;
};

/**
 * A helper function for checking the Bool RAM plugin type.
 *
 * The ram type has the form
 *
 *   @plugin(ram_bool_v0, ram,
 *       <idx-type>, <idx-bits>, <elt-bits>,
 *       <num-allocs>, <total-alloc>, <max-alloc>)
 *
 *  - idx-type:    the type index for the buffer's index/element type.
 *  - idx-bits:    number of bits used for RAM indexing.
 *  - elt-bits:    number of bits used for RAM buffer elements.
 *  - num-allocs:  the number of allocations made by the circuit.
 *  - total-alloc: the sum/total size of all allocations.
 *  - max-alloc:   the maximum total size of concurrently allocated buffers.
 */
template<typename Number_T>
bool checkBoolRAMType(wtk::circuit::TypeSpec<Number_T> const* const type,
    wtk::type_idx* const idx_type, wtk::wire_idx* const idx_bits,
    wtk::wire_idx* const elt_bits, wtk::wire_idx* const num_allocs,
    wtk::wire_idx* const total_allocs, wtk::wire_idx* const max_alloc);


/** ==== Fallback implementation of RAM ==== */

template<typename Wire_T>
struct FallbackBoolRAMBuffer
{
  size_t size = 0;
  size_t raw_size = 0;

  Wire_T* raw_buffer;

  std::vector<std::vector<Wire_T const*>> buffer;

  FallbackBoolRAMBuffer() = default;

  FallbackBoolRAMBuffer(FallbackBoolRAMBuffer const& copy) = delete;
  FallbackBoolRAMBuffer& operator=(FallbackBoolRAMBuffer const& copy) = delete;

  FallbackBoolRAMBuffer(FallbackBoolRAMBuffer&& move) = default;
  FallbackBoolRAMBuffer& operator=(FallbackBoolRAMBuffer&& move) = default;

  ~FallbackBoolRAMBuffer();
};

template<typename Number_T, typename Wire_T>
struct FallbackBoolRAMBackend
 : public BoolRAMBackend<Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>
{
  FallbackBoolRAMBackend(wtk::circuit::TypeSpec<Number_T> const* const tp,
      wtk::type_idx const wt, wtk::TypeBackend<Number_T, Wire_T>* const wb,
      wtk::wire_idx const ib, wtk::wire_idx const eb)
    : BoolRAMBackend<Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>(
        tp, wt, wb, ib, eb) { }

  bool check() override;
};

template<typename Number_T, typename Wire_T>
struct FallbackBoolRAMInitOperation
  : public BoolRAMInitOperation<Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>
{
  FallbackBoolRAMInitOperation(wtk::type_idx const t,
      BoolRAMBackend<Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>* const be)
    : BoolRAMInitOperation<Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>(t, be) { }

  void init(wtk::wire_idx const size,
      FallbackBoolRAMBuffer<Wire_T>* const buffer,
      Wire_T const* const fill) override;
};

template<typename Number_T, typename Wire_T>
struct FallbackBoolRAMReadOperation
  : public BoolRAMReadOperation<Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>
{
  StrictTreedBooleanMuxOperation<Number_T, Wire_T> BoolMuxOperation;

  FallbackBoolRAMReadOperation(wtk::type_idx const t,
      BoolRAMBackend<Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>* const be)
    : BoolRAMReadOperation<Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>(t, be), BoolMuxOperation(this->type, this->ramBackend()->wireBackend) {
       }

  void read(Wire_T* const out,
      FallbackBoolRAMBuffer<Wire_T>* const buffer,
      Wire_T const* const idx) override;
};

template<typename Number_T, typename Wire_T>
struct FallbackBoolRAMWriteOperation
  : public BoolRAMWriteOperation<Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>
{
  FallbackBoolRAMWriteOperation(wtk::type_idx const t,
      BoolRAMBackend<Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>* const be)
    : BoolRAMWriteOperation<Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>(t, be) { }

  void write(FallbackBoolRAMBuffer<Wire_T>* const buffer,
      Wire_T const* const idx, Wire_T const* const in) override;
};

template<typename Number_T, typename Wire_T>
struct FallbackBoolRAMPlugin
  : public BoolRAMPlugin<Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>
{

  BoolRAMInitOperation<
    Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>* buildInitOperation(
        wtk::type_idx const type, BoolRAMBackend<
          Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>* const be) override;

  BoolRAMReadOperation<
    Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>* buildReadOperation(
        wtk::type_idx const type, BoolRAMBackend<
          Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>* const be) override;

  BoolRAMWriteOperation<
    Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>* buildWriteOperation(
        wtk::type_idx const type, BoolRAMBackend<
          Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>* const be) override;
};

} } // namespace wtk::plugins

#define LOG_IDENTIFIER "ram_bool_v0"
#include <stealth_logging.h>

#include <wtk/plugins/BooleanRAM.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_PLUGINS_BOOLEAN_RAM_PLUGIN_H_
