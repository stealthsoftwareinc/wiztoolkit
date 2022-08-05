/**
 * Copyright (C) 2020-2021 Stealth Software Technologies, Inc.
 */

namespace wtk {

template<typename Number_T>
void ArithmeticStreamHandler<Number_T>::setLineNum(size_t const line)
{
  (void) line;
}

template<typename Number_T>
void ArithmeticStreamHandler<Number_T>::handleInstance(wtk::index_t const idx)
{
  (void) idx;
}

template<typename Number_T>
void ArithmeticStreamHandler<Number_T>::handleShortWitness(
    wtk::index_t const idx)
{
  (void) idx;
}

template<typename Number_T>
void ArithmeticStreamHandler<Number_T>::handleAdd(
    wtk::index_t const out, wtk::index_t const left, wtk::index_t const right)
{
  (void) out;
  (void) left;
  (void) right;
}

template<typename Number_T>
void ArithmeticStreamHandler<Number_T>::handleMul(
    wtk::index_t const out, wtk::index_t const left, wtk::index_t const right)
{
  (void) out;
  (void) left;
  (void) right;
}

template<typename Number_T>
void ArithmeticStreamHandler<Number_T>::handleAddC(
    wtk::index_t const out, wtk::index_t const left, Number_T const right)
{
  (void) out;
  (void) left;
  (void) right;
}

template<typename Number_T>
void ArithmeticStreamHandler<Number_T>::handleMulC(
    wtk::index_t const out, wtk::index_t const left, Number_T const right)
{
  (void) out;
  (void) left;
  (void) right;
}

template<typename Number_T>
void ArithmeticStreamHandler<Number_T>::handleAssign(
    wtk::index_t const out, Number_T const val)
{
  (void) out;
  (void) val;
}

template<typename Number_T>
void ArithmeticStreamHandler<Number_T>::handleCopy(
    wtk::index_t const out, wtk::index_t const in)
{
  (void) out;
  (void) in;
}

template<typename Number_T>
void ArithmeticStreamHandler<Number_T>::handleAssertZero(wtk::index_t const in)
{
  (void) in;
}

template<typename Number_T>
void ArithmeticStreamHandler<Number_T>::handleDeleteSingle(wtk::index_t const in)
{
  (void) in;
}

template<typename Number_T>
void ArithmeticStreamHandler<Number_T>::handleDeleteRange(
    wtk::index_t const first, wtk::index_t const last)
{
  (void) first;
  (void) last;
}

template<typename Number_T>
void ArithmeticStreamHandler<Number_T>::handleEnd() { }

} // namespace wtk
