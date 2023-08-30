/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace plugins {

template<typename Number_T, typename Wire_T>
bool ComparisonOperation<Number_T, Wire_T>::checkSignature(
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  if(signature->outputs.size() != 1 || signature->outputs[0].length != 1)
  {
    log_error("In %s plugin function %s, expected 1 output range with 1 item",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  if(signature->inputs.size() != 2 || signature->inputs[0].length != 1
      || signature->inputs[1].length != 1)
  {
    log_error(
        "In %s plugin function %s, expected 2 input ranges with 1 item each",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  if(signature->outputs[0].type != this->type)
  {
    log_error("in %s plugin function %s, non-matching output type %d",
        binding->name.c_str(), signature->name.c_str(),
        (int) signature->outputs[0].type);
    return false;
  }

  if(signature->inputs[0].type != this->type)
  {
    log_error("in %s plugin function %s, non-matching left input type %d",
        binding->name.c_str(), signature->name.c_str(),
        (int) signature->inputs[0].type);
    return false;
  }

  if(signature->inputs[1].type != this->type)
  {
    log_error("in %s plugin function %s, non-matching right input type %d",
        binding->name.c_str(), signature->name.c_str(),
        (int) signature->inputs[1].type);
    return false;
  }

  if(binding->parameters.size() != 0)
  {
    log_error("In %s plugin function %s, not expecting any plugin parameters",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  return true;
}

template<typename Number_T, typename Wire_T>
void ComparisonOperation<Number_T, Wire_T>::evaluate(
    std::vector<WiresRefEraser>& outputs,
    std::vector<WiresRefEraser>& inputs)
{
  log_assert(outputs.size() == 1 && inputs.size() == 2);
  log_assert(outputs[0].type == this->type && outputs[0].size == 1);
  log_assert(inputs[0].type == this->type && inputs[0].size == 1);
  log_assert(inputs[1].type == this->type && inputs[1].size == 1);

  return this->evaluateCmp(static_cast<WiresRef<Wire_T>*>(&outputs[0])->get(),
      static_cast<WiresRef<Wire_T>*>(&inputs[0])->get(),
      static_cast<WiresRef<Wire_T>*>(&inputs[1])->get());
}

template<typename Number_T, typename Wire_T>
bool DivisionOperation<Number_T, Wire_T>::checkSignature(
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  if(signature->outputs.size() != 2 || signature->outputs[0].length != 1
      || signature->outputs[1].length != 1)
  {
    log_error(
        "In %s plugin function %s, expected 2 output ranges with 1 item each",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  if(signature->inputs.size() != 2 || signature->inputs[0].length != 1
      || signature->inputs[1].length != 1)
  {
    log_error(
        "In %s plugin function %s, expected 2 input ranges with 1 item each",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  if(signature->outputs[0].type != this->type)
  {
    log_error("in %s plugin function %s, non-matching quotient output type %d",
        binding->name.c_str(), signature->name.c_str(),
        (int) signature->outputs[0].type);
    return false;
  }

  if(signature->outputs[1].type != this->type)
  {
    log_error(
        "in %s plugin function %s, non-matching remainder output type %d",
        binding->name.c_str(), signature->name.c_str(),
        (int) signature->outputs[1].type);
    return false;
  }

  if(signature->inputs[0].type != this->type)
  {
    log_error("in %s plugin function %s, non-matching left input type %d",
        binding->name.c_str(), signature->name.c_str(),
        (int) signature->inputs[0].type);
    return false;
  }

  if(signature->inputs[1].type != this->type)
  {
    log_error("in %s plugin function %s, non-matching right input type %d",
        binding->name.c_str(), signature->name.c_str(),
        (int) signature->inputs[1].type);
    return false;
  }

  if(binding->parameters.size() != 0)
  {
    log_error("In %s plugin function %s, not expecting any plugin parameters",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  return true;
}

template<typename Number_T, typename Wire_T>
void DivisionOperation<Number_T, Wire_T>::evaluate(
    std::vector<WiresRefEraser>& outputs,
    std::vector<WiresRefEraser>& inputs)
{
  log_assert(outputs.size() == 2 && inputs.size() == 2);
  log_assert(outputs[0].type == this->type && outputs[0].size == 1);
  log_assert(outputs[1].type == this->type && outputs[1].size == 1);
  log_assert(inputs[0].type == this->type && inputs[0].size == 1);
  log_assert(inputs[1].type == this->type && inputs[1].size == 1);

  return this->evaluateDiv(
      static_cast<WiresRef<Wire_T>*>(&outputs[0])->get(),
      static_cast<WiresRef<Wire_T>*>(&outputs[1])->get(),
      static_cast<WiresRef<Wire_T>*>(&inputs[0])->get(),
      static_cast<WiresRef<Wire_T>*>(&inputs[1])->get());
}

template<typename Number_T, typename Wire_T>
bool BitDecomposeOperation<Number_T, Wire_T>::checkSignature(
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  if(signature->outputs.size() != 1
      || signature->outputs[0].length != this->bits)
  {
    log_error(
        "In %s plugin function %s, expected 1 output range with %zu items",
        binding->name.c_str(), signature->name.c_str(), this->bits);
    return false;
  }

  if(signature->inputs.size() != 1 || signature->inputs[0].length != 1)
  {
    log_error(
        "In %s plugin function %s, expected 1 input range with 1 item",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  if(signature->outputs[0].type != this->type)
  {
    log_error("in %s plugin function %s, non-matching output type %d",
        binding->name.c_str(), signature->name.c_str(),
        (int) signature->outputs[0].type);
    return false;
  }

  if(signature->inputs[0].type != this->type)
  {
    log_error("in %s plugin function %s, non-matching input type %d",
        binding->name.c_str(), signature->name.c_str(),
        (int) signature->inputs[0].type);
    return false;
  }

  if(binding->parameters.size() != 0)
  {
    log_error("In %s plugin function %s, not expecting any plugin parameters",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  return true;
}

template<typename Number_T, typename Wire_T>
void BitDecomposeOperation<Number_T, Wire_T>::evaluate(
    std::vector<WiresRefEraser>& outputs,
    std::vector<WiresRefEraser>& inputs)
{
  log_assert(outputs.size() == 1 && inputs.size() == 1);
  log_assert(outputs[0].type == this->type && outputs[0].size == this->bits);
  log_assert(inputs[0].type == this->type && inputs[0].size == 1);

  return this->evaluateDecomp(
      static_cast<WiresRef<Wire_T>*>(&outputs[0])->get(),
      static_cast<WiresRef<Wire_T>*>(&inputs[0])->get());
}

template<typename Number_T, typename Wire_T>
bool ExtendedArithmeticPlugin<Number_T, Wire_T>::buildBackend(
    wtk::type_idx const type,
    wtk::TypeBackend<Number_T, Wire_T>* const backend,
    wtk::utils::CharMap<std::unique_ptr<
      SimpleOperation<Number_T, Wire_T>>>* const operations)
{
  // Create an less than operation
  {
    auto lt_op = this->buildLessThan(type, backend);
    if(lt_op == nullptr) { return false; }
    operations->emplace("less_than", lt_op);
  }

  // Create an less than equal operation
  {
    auto lte_op = this->buildLessThanEqual(type, backend);
    if(lte_op == nullptr) { return false; }
    operations->emplace("less_than_equal", lte_op);
  }

  // Create a division operation
  {
    auto div_op = this->buildDivision(type, backend);
    if(div_op == nullptr) { return false; }
    operations->emplace("division", div_op);
  }

  // Create a decompose operation
  {
    auto decomp_op = this->buildBitDecompose(type, backend);
    if(decomp_op == nullptr) { return false; }
    operations->emplace("bit_decompose", decomp_op);
  }

  return true;
}

template<typename Number_T>
void pub_decomp(std::vector<Number_T>& dcmp, Number_T const& num)
{
  log_assert(num > 0);

  Number_T n = num;

  // Decompose little endian
  while(n != 0) { dcmp.emplace_back(n & 1); n = n >> 1; }

  // and reverse
  size_t h = dcmp.size() - 1;
  size_t l = 0;
  while (h > l)
  {
    Number_T tmp = dcmp[h];
    dcmp[h] = dcmp[l];
    dcmp[l] = tmp;
    h--;
    l++;
  }
}

template<typename Number_T, typename Wire_T>
ALWAYS_INLINE static inline
void bit_and(Wire_T* const o, Wire_T const* const l, Wire_T const* const r,
    wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  backend->mulGate(o, l, r);
}

template<typename Number_T, typename Wire_T>
ALWAYS_INLINE static inline
void bit_andc(Wire_T* const o, Wire_T const* const l, Number_T const& r,
    wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  backend->mulcGate(o, l, Number_T(r));
}

template<typename Number_T, typename Wire_T>
ALWAYS_INLINE static inline
void bit_ior(Wire_T* const o, Wire_T const* const l, Wire_T const* const r,
    Number_T const& neg1, wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  Wire_T tmp_add;
  backend->addGate(&tmp_add, l, r);
  Wire_T tmp_mul;
  backend->mulGate(&tmp_mul, l, r);
  Wire_T tmp_neg_mul;
  backend->mulcGate(&tmp_neg_mul, &tmp_mul, Number_T(neg1));
  backend->addGate(o, &tmp_add, &tmp_neg_mul);
}

template<typename Number_T, typename Wire_T>
ALWAYS_INLINE static inline
void bit_iorc(Wire_T* const o, Wire_T const* const l, Number_T const& r,
    Number_T const& neg1, wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  Wire_T tmp_add;
  backend->addcGate(&tmp_add, l, Number_T(r));
  Wire_T tmp_mul;
  backend->mulcGate(&tmp_mul, l, Number_T(r));
  Wire_T tmp_neg_mul;
  backend->mulcGate(&tmp_neg_mul, &tmp_mul, Number_T(neg1));
  backend->addGate(o, &tmp_add, &tmp_neg_mul);
}

template<typename Number_T, typename Wire_T>
ALWAYS_INLINE static inline
void bit_xor(Wire_T* const o, Wire_T const* const l, Wire_T const* const r,
    Number_T const& neg1, wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  Wire_T tmp_add;
  backend->addGate(&tmp_add, l, r);
  Wire_T tmp_mul;
  backend->mulGate(&tmp_mul, l, r);
  Wire_T tmp_neg_mul;
  backend->mulcGate(&tmp_neg_mul, &tmp_mul, Number_T(neg1 - 1));
  backend->addGate(o, &tmp_add, &tmp_neg_mul);
}

template<typename Number_T, typename Wire_T>
ALWAYS_INLINE static inline
void bit_xorc(Wire_T* const o, Wire_T const* const l, Number_T const& r,
    Number_T const& neg1, wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  Wire_T tmp_add;
  backend->addcGate(&tmp_add, l, Number_T(r));
  Wire_T tmp_mul;
  backend->mulcGate(&tmp_mul, l, Number_T(r));
  Wire_T tmp_neg_mul;
  backend->mulcGate(&tmp_neg_mul, &tmp_mul, Number_T(neg1 - 1));
  backend->addGate(o, &tmp_add, &tmp_neg_mul);
}

template<typename Number_T, typename Wire_T>
ALWAYS_INLINE static inline
void bit_not(Wire_T* const o, Wire_T const* const i,
    Number_T const& neg1, wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  Wire_T tmp_mul;
  backend->mulcGate(&tmp_mul, i, Number_T(neg1));
  backend->addcGate(o, &tmp_mul, Number_T(1));
}

template<typename Number_T>
Number_T bit_notc(Number_T const& i)
{
  return (i + 1) & 1;
}

template<typename Number_T, typename Wire_T>
ALWAYS_INLINE static inline
void bits_lt_comparator(Wire_T* const out,
    Wire_T const* const l_dcmp, Wire_T const* const r_dcmp,
    Number_T const& neg1, size_t const bits,
    wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  Wire_T tmp1;
  Wire_T tmp2;
  bit_not(&tmp1, &l_dcmp[0], neg1, backend);
  Wire_T lt[2];
  size_t lt_idx = 0;
  bit_and(&lt[0], &tmp1, &r_dcmp[0], backend);

  Wire_T prev_eq[2];
  bit_xor(&tmp1, &l_dcmp[0], &r_dcmp[0], neg1, backend);
  bit_not(&prev_eq[0], &tmp1, neg1, backend);

  for(size_t i = 1; i < bits - 1; i++)
  {
    size_t next_lt_idx = (lt_idx + 1) & 1;

    bit_not(&tmp1, &l_dcmp[i], neg1, backend);
    bit_and(&tmp2, &tmp1, &r_dcmp[i], backend);
    bit_and(&tmp1, &tmp2, &prev_eq[lt_idx], backend);
    backend->addGate(&lt[next_lt_idx], &tmp1, &lt[lt_idx]);

    bit_xor(&tmp1, &l_dcmp[i], &r_dcmp[i], neg1, backend);
    bit_not(&tmp2, &tmp1, neg1, backend);
    bit_and(&prev_eq[next_lt_idx], &prev_eq[lt_idx], &tmp2, backend);

    lt_idx = next_lt_idx;
  }

  bit_not(&tmp1, &l_dcmp[bits - 1], neg1, backend);
  bit_and(&tmp2, &tmp1, &r_dcmp[bits - 1], backend);
  bit_and(&tmp1, &tmp2, &prev_eq[lt_idx], backend);
  backend->addGate(out, &tmp1, &lt[lt_idx]);
}

template<typename Number_T, typename Wire_T>
ALWAYS_INLINE static inline
void bits_ltc_comparator(Wire_T* const out,
    std::vector<Number_T> const& l_dcmp, Wire_T const* const r_dcmp,
    Number_T const& neg1, size_t const bits,
    wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  Wire_T tmp1;
  Wire_T tmp2;
  Wire_T lt[2];
  size_t lt_idx = 0;
  bit_andc(&lt[0], &r_dcmp[0], bit_notc(l_dcmp[0]), backend);

  Wire_T prev_eq[2];
  bit_xorc(&tmp1, &r_dcmp[0], l_dcmp[0], neg1, backend);
  bit_not(&prev_eq[0], &tmp1, neg1, backend);

  for(size_t i = 1; i < bits - 1; i++)
  {
    size_t next_lt_idx = (lt_idx + 1) & 1;

    bit_andc(&tmp2, &r_dcmp[i], bit_notc(l_dcmp[i]), backend);
    bit_and(&tmp1, &tmp2, &prev_eq[lt_idx], backend);
    backend->addGate(&lt[next_lt_idx], &tmp1, &lt[lt_idx]);

    bit_xorc(&tmp1, &r_dcmp[i], l_dcmp[i], neg1, backend);
    bit_not(&tmp2, &tmp1, neg1, backend);
    bit_and(&prev_eq[next_lt_idx], &prev_eq[lt_idx], &tmp2, backend);

    lt_idx = next_lt_idx;
  }

  bit_andc(&tmp2, &r_dcmp[bits - 1], bit_notc(l_dcmp[bits - 1]), backend);
  bit_and(&tmp1, &tmp2, &prev_eq[lt_idx], backend);
  backend->addGate(out, &tmp1, &lt[lt_idx]);
}

template<typename Number_T, typename Wire_T>
ALWAYS_INLINE static inline
void bits_lte_comparator(Wire_T* const out,
    Wire_T const* const l_dcmp, Wire_T const* const r_dcmp,
    Number_T const& neg1, size_t const bits,
    wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  Wire_T tmp1;
  Wire_T tmp2;
  bit_not(&tmp1, &l_dcmp[0], neg1, backend);
  Wire_T lt[2];
  size_t lt_idx = 0;
  bit_and(&lt[0], &tmp1, &r_dcmp[0], backend);

  Wire_T prev_eq[2];
  bit_xor(&tmp1, &l_dcmp[0], &r_dcmp[0], neg1, backend);
  bit_not(&prev_eq[0], &tmp1, neg1, backend);

  for(size_t i = 1; i < bits - 1; i++)
  {
    size_t next_lt_idx = (lt_idx + 1) & 1;

    bit_not(&tmp1, &l_dcmp[i], neg1, backend);
    bit_and(&tmp2, &tmp1, &r_dcmp[i], backend);
    bit_and(&tmp1, &tmp2, &prev_eq[lt_idx], backend);
    backend->addGate(&lt[next_lt_idx], &tmp1, &lt[lt_idx]);

    bit_xor(&tmp1, &l_dcmp[i], &r_dcmp[i], neg1, backend);
    bit_not(&tmp2, &tmp1, neg1, backend);
    bit_and(&prev_eq[next_lt_idx], &prev_eq[lt_idx], &tmp2, backend);

    lt_idx = next_lt_idx;
  }

  size_t next_lt_idx = (lt_idx + 1) & 1;

  bit_not(&tmp1, &l_dcmp[bits - 1], neg1, backend);
  bit_and(&tmp2, &tmp1, &r_dcmp[bits - 1], backend);
  bit_and(&tmp1, &tmp2, &prev_eq[lt_idx], backend);
  backend->addGate(&lt[next_lt_idx], &tmp1, &lt[lt_idx]);

  bit_xor(&tmp1, &l_dcmp[bits - 1], &r_dcmp[bits - 1], neg1, backend);
  bit_not(&tmp2, &tmp1, neg1, backend);
  bit_and(&prev_eq[next_lt_idx], &prev_eq[lt_idx], &tmp2, backend);

  backend->addGate(out, &lt[next_lt_idx], &prev_eq[next_lt_idx]);
}

template<typename Number_T, typename Wire_T>
ALWAYS_INLINE static inline
void bits_ltec_comparator(Wire_T* const out,
    std::vector<Number_T> const& l_dcmp, Wire_T const* const r_dcmp,
    Number_T const& neg1, size_t const bits,
    wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  Wire_T tmp1;
  Wire_T tmp2;
  Wire_T lt[2];
  size_t lt_idx = 0;
  bit_andc(&lt[0], &r_dcmp[0], bit_notc(l_dcmp[0]), backend);

  Wire_T prev_eq[2];
  bit_xorc(&tmp1, &r_dcmp[0], l_dcmp[0], neg1, backend);
  bit_not(&prev_eq[0], &tmp1, neg1, backend);

  for(size_t i = 1; i < bits - 1; i++)
  {
    size_t next_lt_idx = (lt_idx + 1) & 1;

    bit_andc(&tmp2, &r_dcmp[i], bit_notc(l_dcmp[i]), backend);
    bit_and(&tmp1, &tmp2, &prev_eq[lt_idx], backend);
    backend->addGate(&lt[next_lt_idx], &tmp1, &lt[lt_idx]);

    bit_xorc(&tmp1, &r_dcmp[i], l_dcmp[i], neg1, backend);
    bit_not(&tmp2, &tmp1, neg1, backend);
    bit_and(&prev_eq[next_lt_idx], &prev_eq[lt_idx], &tmp2, backend);

    lt_idx = next_lt_idx;
  }

  size_t next_lt_idx = (lt_idx + 1) & 1;

  bit_andc(&tmp2, &r_dcmp[bits - 1], bit_notc(l_dcmp[bits - 1]), backend);
  bit_and(&tmp1, &tmp2, &prev_eq[lt_idx], backend);
  backend->addGate(&lt[next_lt_idx], &tmp1, &lt[lt_idx]);

  bit_xorc(&tmp1, &r_dcmp[bits - 1], l_dcmp[bits - 1], neg1, backend);
  bit_not(&tmp2, &tmp1, neg1, backend);
  bit_and(&prev_eq[next_lt_idx], &prev_eq[lt_idx], &tmp2, backend);

  backend->addGate(out, &lt[next_lt_idx], &prev_eq[next_lt_idx]);
}

// little end first, big end last.
template<typename Number_T, typename Wire_T>
void bit_decomp(
    Wire_T const* const input, Number_T const& input_num, Wire_T* const dcmp,
    Number_T const& neg1, std::vector<Number_T> const& prime_bits,
    wtk::TypeBackend<Number_T, Wire_T>* const backend,
    wtk::utils::Setting const setting)
{
  Number_T cpy = input_num;

  // This is an attempted cheat which is caught by the lte comparison below
  // Number_T t = (Number_T(1) << prime_bits.size()) - 1 - backend->type->prime;
  // if(cpy < t) { cpy += backend->type->prime; }

  {
    Wire_T bit_inv, zero;

    for(size_t i = 1; i <= prime_bits.size(); i++)
    {
      if(setting == wtk::utils::Setting::prover)
      {
        // prover decomposes to bits, as witness values
        Number_T bit = cpy & 1;
        cpy = cpy >> 1;

        backend->privateIn(&dcmp[prime_bits.size() - i], std::move(bit));
      }
      else
      {
        backend->privateIn(&dcmp[prime_bits.size() - i], Number_T(0));
      }

      // Check that the bit is in fact a bit
      backend->addcGate(&bit_inv, &dcmp[prime_bits.size() - i], Number_T(neg1));
      backend->mulGate(&zero, &bit_inv, &dcmp[prime_bits.size() - i]);
      backend->assertZero(&zero);
    }
  }

  // Prove the recomposition is correct
  {
    Wire_T recomp;
    backend->copy(&recomp, &dcmp[0]);

    Wire_T tmp;

    for(size_t i = 1; i < prime_bits.size(); i++)
    {
      backend->mulcGate(&tmp, &recomp, Number_T(2));
      backend->addGate(&recomp, &tmp, &dcmp[i]);
    }

    backend->mulcGate(&tmp, &recomp, Number_T(neg1));

    Wire_T zero;
    backend->addGate(&zero, &tmp, input);
    backend->assertZero(&zero);
  }

  // prove the recomposition does not overflow.
  // So, like assert_zero(prime_bits <= dcmp)
  {
    Wire_T lt;
    bits_ltc_comparator(
        &lt, prime_bits, dcmp, neg1, prime_bits.size(), backend);
    backend->assertZero(&lt);
  }
}

template<typename Number_T, typename Wire_T>
FallbackLessThanOperation<Number_T, Wire_T>::FallbackLessThanOperation(
    wtk::type_idx const t, wtk::TypeBackend<Number_T, Wire_T>* const be,
    std::vector<Number_T>&& pd, wtk::utils::Setting s)
  : ComparisonOperation<Number_T, Wire_T>(t, be), primeDecomp(std::move(pd)),
    setting(s)
{
}

template<typename Number_T, typename Wire_T>
void FallbackLessThanOperation<Number_T, Wire_T>::evaluateCmp(
    Wire_T* const out, Wire_T const* const left, Wire_T const* const right)
{
  Number_T const l_num = this->backend->getExtendedWitness(left);
  Number_T const r_num = this->backend->getExtendedWitness(right);

  std::vector<Wire_T> l_dcmp(this->primeDecomp.size());
  std::vector<Wire_T> r_dcmp(this->primeDecomp.size());

  Number_T const neg1 = this->backend->type->prime - 1;

  bit_decomp(left, l_num, &l_dcmp[0],
      neg1, this->primeDecomp, this->backend, this->setting);
  bit_decomp(right, r_num, &r_dcmp[0],
      neg1, this->primeDecomp, this->backend, this->setting);

  bits_lt_comparator(out, &l_dcmp[0], &r_dcmp[0],
      neg1, this->primeDecomp.size(), this->backend);
}

template<typename Number_T, typename Wire_T>
FallbackLessThanEqualOperation<Number_T, Wire_T>
::FallbackLessThanEqualOperation(
    wtk::type_idx const t, wtk::TypeBackend<Number_T, Wire_T>* const be,
    std::vector<Number_T>&& pd, wtk::utils::Setting s)
  : ComparisonOperation<Number_T, Wire_T>(t, be), primeDecomp(std::move(pd)),
    setting(s)
{
}

template<typename Number_T, typename Wire_T>
void FallbackLessThanEqualOperation<Number_T, Wire_T>::evaluateCmp(
    Wire_T* const out, Wire_T const* const left, Wire_T const* const right)
{
  Number_T const l_num = this->backend->getExtendedWitness(left);
  Number_T const r_num = this->backend->getExtendedWitness(right);

  std::vector<Wire_T> l_dcmp(this->primeDecomp.size());
  std::vector<Wire_T> r_dcmp(this->primeDecomp.size());

  Number_T const neg1 = this->backend->type->prime - 1;

  bit_decomp(left, l_num, &l_dcmp[0],
      neg1, this->primeDecomp, this->backend, this->setting);
  bit_decomp(right, r_num, &r_dcmp[0],
      neg1, this->primeDecomp, this->backend, this->setting);

  bits_lte_comparator(out, &l_dcmp[0], &r_dcmp[0],
      neg1, this->primeDecomp.size(), this->backend);
}

template<typename Number_T, typename Wire_T>
FallbackDivisionOperation<Number_T, Wire_T>::FallbackDivisionOperation(
    wtk::type_idx const t, wtk::TypeBackend<Number_T, Wire_T>* const be,
    std::vector<Number_T>&& pd, wtk::utils::Setting s)
  : DivisionOperation<Number_T, Wire_T>(t, be), primeDecomp(std::move(pd)),
    setting(s)
{
}

template<typename Number_T, typename Wire_T>
void FallbackDivisionOperation<Number_T, Wire_T>::evaluateDiv(
    Wire_T* const quotient, Wire_T* const remainder,
    Wire_T const* const left, Wire_T const* const right)
{
  Number_T l_num = this->backend->getExtendedWitness(left);
  Number_T r_num = this->backend->getExtendedWitness(right);
  Number_T q_num = 0;
  Number_T m_num = 0;
  // compute the quotient in plain
  if(this->setting == wtk::utils::Setting::prover)
  {
    q_num = l_num / r_num;
    m_num = l_num % r_num;

    this->backend->privateIn(quotient, Number_T(q_num));
    this->backend->privateIn(remainder, Number_T(m_num));
  }
  else
  {
    this->backend->privateIn(quotient, Number_T(0));
    this->backend->privateIn(remainder, Number_T(0));
  }

  Number_T const neg1 = this->backend->type->prime - 1;

  // prove that the quotient and remainder add up to the dividend
  Wire_T mul;
  this->backend->mulGate(&mul, quotient, right);
  Wire_T add;
  this->backend->addGate(&add, &mul, remainder);
  Wire_T neg;
  this->backend->mulcGate(&neg, &add, Number_T(neg1));
  this->backend->addGate(&add, &neg, left);
  this->backend->assertZero(&add);

  // check that the quotient is less than the divisor
  // like @assertZero(r_dcmp <= m_dcmp)
  std::vector<Wire_T> r_dcmp(this->primeDecomp.size());
  std::vector<Wire_T> m_dcmp(this->primeDecomp.size());

  bit_decomp(right, r_num, &r_dcmp[0],
      neg1, this->primeDecomp, this->backend, this->setting);
  bit_decomp(remainder, m_num, &m_dcmp[0],
      neg1, this->primeDecomp, this->backend, this->setting);

  Wire_T zero;
  bits_lte_comparator(&zero, &r_dcmp[0], &m_dcmp[0],
      neg1, this->primeDecomp.size(), this->backend);
  this->backend->assertZero(&zero);
}

template<typename Number_T, typename Wire_T>
FallbackBitDecomposeOperation<Number_T, Wire_T>::FallbackBitDecomposeOperation(
    wtk::type_idx const t, wtk::TypeBackend<Number_T, Wire_T>* const be,
    size_t b, std::vector<Number_T>&& pd, wtk::utils::Setting s)
  : BitDecomposeOperation<Number_T, Wire_T>(t, be, b),
    primeDecomp(std::move(pd)), setting(s)
{
}

template<typename Number_T, typename Wire_T>
void FallbackBitDecomposeOperation<Number_T, Wire_T>::evaluateDecomp(
    Wire_T* const out_bits, Wire_T const* const input)
{
  Number_T const num = this->backend->getExtendedWitness(input);
  Number_T const neg1 = this->backend->type->prime - 1;
  bit_decomp(input, num, out_bits,
      neg1, this->primeDecomp, this->backend, this->setting);
}

template<typename Number_T, typename Wire_T>
ComparisonOperation<Number_T, Wire_T>*
FallbackExtendedArithmeticPlugin<Number_T, Wire_T>::buildLessThan(
    wtk::type_idx const type,
    wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  if(backend->type->variety != wtk::circuit::TypeSpec<Number_T>::field)
  {
    log_error("Only prime field types are currently compatible with the "
        "extended_arithmetic plugin.");
    return nullptr;
  }

  std::vector<Number_T> prime_decomp;
  pub_decomp(prime_decomp, backend->type->prime);
  if(backend->supportsExtendedWitness() && prime_decomp.size() >= 2)
  {
    return new FallbackLessThanOperation<Number_T, Wire_T>(
        type, backend, std::move(prime_decomp), this->setting);
  }
  else
  {
    log_error("A backend must support the extended witness API to function "
        "with the extended_arithmetic plugin and the prime must be 2 or more "
        "bits wide.");
    return nullptr;
  }
}

template<typename Number_T, typename Wire_T>
ComparisonOperation<Number_T, Wire_T>*
FallbackExtendedArithmeticPlugin<Number_T, Wire_T>::buildLessThanEqual(
    wtk::type_idx const type,
    wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  if(backend->type->variety != wtk::circuit::TypeSpec<Number_T>::field)
  {
    log_error("Only prime field types are currently compatible with the "
        "extende_arithmetic plugin.");
    return nullptr;
  }

  std::vector<Number_T> prime_decomp;
  pub_decomp(prime_decomp, backend->type->prime);
  if(backend->supportsExtendedWitness() && prime_decomp.size() >= 2)
  {
    return new FallbackLessThanEqualOperation<Number_T, Wire_T>(
        type, backend, std::move(prime_decomp), this->setting);
  }
  else
  {
    log_error("A backend must support the extended witness API to function "
        "with the extended_arithmetic plugin.");
    return nullptr;
  }
}

template<typename Number_T, typename Wire_T>
DivisionOperation<Number_T, Wire_T>*
FallbackExtendedArithmeticPlugin<Number_T, Wire_T>::buildDivision(
    wtk::type_idx const type,
    wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  if(backend->type->variety != wtk::circuit::TypeSpec<Number_T>::field)
  {
    log_error("Only prime field types are currently compatible with the "
        "extended_arithmetic plugin.");
    return nullptr;
  }

  std::vector<Number_T> prime_decomp;
  pub_decomp(prime_decomp, backend->type->prime);
  if(backend->supportsExtendedWitness() && prime_decomp.size() >= 2)
  {
    return new FallbackDivisionOperation<Number_T, Wire_T>(
        type, backend, std::move(prime_decomp), this->setting);
  }
  else
  {
    log_error("A backend must support the extended witness API to function "
        "with the extended_arithmetic plugin.");
    return nullptr;
  }
}

template<typename Number_T, typename Wire_T>
BitDecomposeOperation<Number_T, Wire_T>*
FallbackExtendedArithmeticPlugin<Number_T, Wire_T>::buildBitDecompose(
    wtk::type_idx const type,
    wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  if(backend->type->variety != wtk::circuit::TypeSpec<Number_T>::field)
  {
    log_error("Only prime field types are currently compatible with the "
        "extended_arithmetic plugin.");
    return nullptr;
  }

  std::vector<Number_T> prime_decomp;
  pub_decomp(prime_decomp, backend->type->prime);
  if(backend->supportsExtendedWitness() && prime_decomp.size() >= 2)
  {
    return new FallbackBitDecomposeOperation<Number_T, Wire_T>(
        type, backend, prime_decomp.size(), std::move(prime_decomp),
        this->setting);
  }
  else
  {
    log_error("A backend must support the extended witness API to function "
        "with the extended_arithmetic plugin.");
    return nullptr;
  }
}

} }
