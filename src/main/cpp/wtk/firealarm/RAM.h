/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

#ifndef WTK_FIREALARM_RAM_PLUGIN_H_
#define WTK_FIREALARM_RAM_PLUGIN_H_

#include <cstddef>
#include <vector>

#include <wtk/indexes.h>
#include <wtk/circuit/Data.h>
#include <wtk/plugins/SimplePlugin.h>
#include <wtk/plugins/ArithmeticRAM.h>
#include <wtk/plugins/BooleanRAM.h>

#include <wtk/firealarm/Counters.h>
#include <wtk/firealarm/FieldBackend.h>

namespace wtk {
namespace firealarm {

// Just a simple length and pointer pair.
template<typename Wire_T>
struct RAMBuffer
{
  size_t length = 0;

  Wire_T* buffer = nullptr;

  TypeCounter* counter = nullptr;

  ~RAMBuffer();
};

template<typename Number_T, typename Wire_T>
struct RAMBackend
  : public wtk::plugins::RAMBackend<Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>
{
  char const* const fileName;
  TypeCounter* const counter;

  RAMBackend(char const* const fn, wtk::type_idx const it,
      wtk::TypeBackend<Number_T, Wire<Wire_T>>* const ibe,
      TypeCounter* const ctr, wtk::circuit::TypeSpec<Number_T> const* const tp)
    : wtk::plugins::RAMBackend<
        Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>(tp, it, ibe),
      fileName(fn), counter(ctr) { }

  bool check() override;

  bool failure = false;
};

template<typename Number_T, typename Wire_T>
struct RAMInitOperation : public wtk::plugins::RAMInitOperation<
    Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>
{
  RAMInitOperation(wtk::type_idx const t,
      wtk::plugins::RAMBackend<
        Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>* const be)
    : wtk::plugins::RAMInitOperation<
        Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>(t, be) { }

  void init(wtk::wire_idx const size,
      RAMBuffer<Wire_T>* const buffer, Wire<Wire_T> const* const fill);
};

template<typename Number_T, typename Wire_T>
struct RAMReadOperation : public wtk::plugins::RAMReadOperation<
    Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>
{
  RAMReadOperation(wtk::type_idx const t,
      wtk::plugins::RAMBackend<
        Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>* const be)
    : wtk::plugins::RAMReadOperation<
        Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>(t, be) { }

  void read(Wire<Wire_T>* const out,
      RAMBuffer<Wire_T>* const buffer, Wire<Wire_T> const* const idx);
};

template<typename Number_T, typename Wire_T>
struct RAMWriteOperation : public wtk::plugins::RAMWriteOperation<
    Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>
{
  RAMWriteOperation(wtk::type_idx const t,
      wtk::plugins::RAMBackend<
        Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>* const be)
    : wtk::plugins::RAMWriteOperation<
        Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>(t, be) { }

  void write(RAMBuffer<Wire_T>* const buffer,
      Wire<Wire_T> const* const idx, Wire<Wire_T> const* const in);
};

template<typename Number_T, typename Wire_T>
struct RAMPlugin
  : public wtk::plugins::RAMPlugin<Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>
{
  RAMInitOperation<Number_T, Wire_T>* buildInitOperation(
      wtk::type_idx const type,
      wtk::plugins::RAMBackend<
        Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>* const be) override;

  RAMReadOperation<Number_T, Wire_T>* buildReadOperation(
      wtk::type_idx const type,
      wtk::plugins::RAMBackend<
        Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>* const be) override;

  RAMWriteOperation<Number_T, Wire_T>* buildWriteOperation(
      wtk::type_idx const type,
      wtk::plugins::RAMBackend<
        Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>* const be) override;
};

template<typename Number_T, typename Wire_T>
struct BoolRAMBackend : public wtk::plugins::BoolRAMBackend<
  Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>
{
  char const* const fileName;
  TypeCounter* const counter;

  BoolRAMBackend(char const* const fn, wtk::type_idx const it,
      wtk::TypeBackend<Number_T, Wire<Wire_T>>* const ibe,
      wtk::wire_idx const ib, wtk::wire_idx const eb,
      TypeCounter* const ctr, wtk::circuit::TypeSpec<Number_T> const* const tp)
    : wtk::plugins::BoolRAMBackend<
        Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>(tp, it, ibe, ib, eb),
      fileName(fn), counter(ctr) { }

  bool check() override;

  bool failure = false;
};

template<typename Number_T, typename Wire_T>
struct BoolRAMInitOperation : public wtk::plugins::BoolRAMInitOperation<
    Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>
{
  BoolRAMInitOperation(wtk::type_idx const t,
      wtk::plugins::BoolRAMBackend<
        Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>* const be)
    : wtk::plugins::BoolRAMInitOperation<
        Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>(t, be) { }

  void init(wtk::wire_idx const size,
      RAMBuffer<Wire_T>* const buffer, Wire<Wire_T> const* const fill);
};

template<typename Number_T, typename Wire_T>
struct BoolRAMReadOperation : public wtk::plugins::BoolRAMReadOperation<
    Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>
{
  BoolRAMReadOperation(wtk::type_idx const t,
      wtk::plugins::BoolRAMBackend<
        Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>* const be)
    : wtk::plugins::BoolRAMReadOperation<
        Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>(t, be) { }

  void read(Wire<Wire_T>* const out,
      RAMBuffer<Wire_T>* const buffer, Wire<Wire_T> const* const idx);
};

template<typename Number_T, typename Wire_T>
struct BoolRAMWriteOperation : public wtk::plugins::BoolRAMWriteOperation<
    Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>
{
  BoolRAMWriteOperation(wtk::type_idx const t,
      wtk::plugins::BoolRAMBackend<
        Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>* const be)
    : wtk::plugins::BoolRAMWriteOperation<
        Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>(t, be) { }

  void write(RAMBuffer<Wire_T>* const buffer,
      Wire<Wire_T> const* const idx, Wire<Wire_T> const* const in);
};

template<typename Number_T, typename Wire_T>
struct BoolRAMPlugin : public wtk::plugins::BoolRAMPlugin<
    Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>
{
  BoolRAMInitOperation<Number_T, Wire_T>* buildInitOperation(
      wtk::type_idx const type,
      wtk::plugins::BoolRAMBackend<
        Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>* const be) override;

  BoolRAMReadOperation<Number_T, Wire_T>* buildReadOperation(
      wtk::type_idx const type,
      wtk::plugins::BoolRAMBackend<
        Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>* const be) override;

  BoolRAMWriteOperation<Number_T, Wire_T>* buildWriteOperation(
      wtk::type_idx const type,
      wtk::plugins::BoolRAMBackend<
        Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>* const be) override;
};

} } // namespace wtk::firealarm

#define LOG_IDENTIFIER "ram_arith_v0"
#include <stealth_logging.h>

#include <wtk/firealarm/RAM.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_FIREALARM_RAM_PLUGIN_H_
