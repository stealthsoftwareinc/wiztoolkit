/**
 * Copyright (C) 2022 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace bolt {

template<typename Wire_T, typename Number_T>
void Backend<Wire_T, Number_T>::exponentiate(
    Wire_T* const out, Wire_T const* const base, Number_T exp)
{
  Wire_T tmp_base[2];
  Wire_T const* base_in_ptr = base;
  size_t base_place = 0;
  Wire_T* base_out_ptr = &tmp_base[base_place];

  Wire_T tmp_aux[2];
  this->assign(&tmp_aux[0], Number_T(1));
  size_t aux_place = 1;

  while(exp > 1)
  {
    if(exp % 2 == 0)
    {
      this->mulGate(base_out_ptr, base_in_ptr, base_in_ptr);

      base_in_ptr = base_out_ptr;
      base_place = (base_place + 1) & 0x01;
      base_out_ptr = &tmp_base[base_place];

      exp = exp / 2;
    }
    else
    {
      size_t const aux_inc = (aux_place + 1) & 0x01;
      this->mulGate(&tmp_aux[aux_place], base_in_ptr, &tmp_aux[aux_inc]);
      this->mulGate(base_out_ptr, base_in_ptr, base_in_ptr);

      aux_place = aux_inc;
      base_in_ptr = base_out_ptr;
      base_place = (base_place + 1) & 0x01;
      base_out_ptr = &tmp_base[base_place];

      exp = (exp - 1) / 2;
    }
  }

  this->mulGate(out, base_in_ptr, &tmp_aux[(aux_place + 1) & 0x01]);
}

template<typename Wire_T, typename Number_T>
void Backend<Wire_T, Number_T>::caseSelect(
    Wire_T* const selected_bit,
    Number_T const case_number,
    Wire_T const* const select_wire)
{
  if(this->isBoolean)
  {
    // case_number is public, came from the relation.
    if(case_number == 0)
    {
      this->notGate(selected_bit, select_wire);
    }
    else // if(case_number == 1)
    {
      this->copy(selected_bit, select_wire);
    }
  }
  else
  {
    Wire_T base;
    this->addcGate(&base, select_wire, this->prime - case_number);
    Wire_T exp;
    this->exponentiate(&exp, &base, this->prime - 1);
    // Reuse base as temp space
    this->mulcGate(&base, &exp, this->prime - 1);
    this->addcGate(selected_bit, &base, Number_T(1));
  }
}

template<typename Wire_T, typename Number_T>
void Backend<Wire_T, Number_T>::multiplexHelper(Wire_T* const out,
    std::vector<LocalWireRange<Wire_T, Number_T>*>* const dummies,
    std::vector<Wire_T> const* const selector_bits,
    wtk::index_t const dummy_place)
{
  if(this->isBoolean)
  {
    log_assert(dummies->size() > 0 && dummies->size() <= 2);

    if(dummies->size() == 1)
    {
      this->andGate(
          out, (*dummies)[0]->deref(dummy_place), &(*selector_bits)[0]);
    }
    else // if(dummies->size() == 2)
    {
      Wire_T tmp0;
      this->andGate(
          &tmp0, (*dummies)[0]->deref(dummy_place), &(*selector_bits)[0]);
      Wire_T tmp1;
      this->andGate(
          &tmp1, (*dummies)[1]->deref(dummy_place), &(*selector_bits)[1]);
      this->xorGate(out, &tmp0, &tmp1);
    }
  }
  else
  {
    log_assert(dummies->size() > 0 && dummies->size() == selector_bits->size());

    if(dummies->size() == 1)
    {
      this->mulGate(out,
          (*dummies)[0]->deref(dummy_place),
          &(*selector_bits)[0]);
    }
    else if(dummies->size() == 2)
    {
      Wire_T tmp0;
      this->mulGate(&tmp0,
          (*dummies)[0]->deref(dummy_place),
          &(*selector_bits)[0]);
      Wire_T tmp1;
      this->mulGate(&tmp1,
          (*dummies)[1]->deref(dummy_place),
          &(*selector_bits)[1]);

      this->addGate(out, &tmp0, &tmp1);
    }
    else
    {
      Wire_T tmp0;
      this->mulGate(&tmp0,
          (*dummies)[0]->deref(dummy_place),
          &(*selector_bits)[0]);
      Wire_T tmp1;
      this->mulGate(&tmp1,
          (*dummies)[1]->deref(dummy_place),
          &(*selector_bits)[1]);

      Wire_T tmpNs[2];
      this->addGate(&tmpNs[0], &tmp0, &tmp1);
      size_t tmp_place = 1;

      for(size_t l = 2; l < dummies->size() - 1; l++)
      {
        Wire_T tmp;
        this->mulGate(&tmp,
            (*dummies)[l]->deref(dummy_place),
            &(*selector_bits)[l]);

        size_t const tmp_inc = (tmp_place + 1) & 0x01;
        this->addGate(&tmpNs[tmp_place], &tmp, &tmpNs[tmp_inc]);
        tmp_place = tmp_inc;
      }

      Wire_T tmpL;
      this->mulGate(&tmpL,
          dummies->back()->deref(dummy_place),
          &selector_bits->back());
      this->addGate(
          out, &tmpNs[(tmp_place + 1) & 0x01], &tmpL);
    }
  }
}

template<typename Wire_T, typename Number_T>
void Backend<Wire_T, Number_T>::checkSelectorBits(
    std::vector<Wire_T> const* const bits, Wire_T const* const enabled_bit)
{
  if(this->isBoolean)
  {
    log_assert(bits->size() > 0 && bits->size() <= 2);

    if(bits->size() == 1)
    {
      Wire_T tmp;
      this->notGate(&tmp, &(*bits)[0]);
      this->assertZero(&tmp);
    }
    // else if(bits->size() == 2)
    // {
    //   // selection is implied by case exclusivity and field coverage.
    //   // (checked elsewhere)
    // }
  }
  else
  {
    Wire_T total_tmps[2];
    Wire_T const* total_wire;

    if(bits->size() == 1)
    {
      total_wire = &(*bits)[0];
    }
    else
    {
      this->addGate(&total_tmps[0], &(*bits)[0], &(*bits)[1]);
      size_t total_place = 1;

      for(size_t j = 2; j < bits->size(); j++)
      {
        size_t const total_inc = (total_place + 1) & 0x01;
        this->addGate(
            &total_tmps[total_place], &total_tmps[total_inc], &(*bits)[j]);
        total_place = total_inc;
      }

      total_wire = &total_tmps[(total_place + 1) & 0x01];
    }

    Wire_T inv_total;
    this->mulcGate(&inv_total, total_wire, this->prime - 1);
    Wire_T zero;
    this->addcGate(&zero, &inv_total, Number_T(1));

    Wire_T* zero_ptr = &zero;
    Wire_T enabled_zero;

    // Check if the containing scope is disabled.
    if(enabled_bit != nullptr)
    {
      this->mulGate(&enabled_zero, enabled_bit, &zero);
      zero_ptr = &enabled_zero;
    }

    this->assertZero(zero_ptr);
  }
}

} } // namespace wtk::bolt
