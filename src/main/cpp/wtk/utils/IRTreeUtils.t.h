/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace utils {

template<typename Number_T>
bool maxInsWit(
    wtk::SwitchStatement<Number_T>* switch_stmt,
    std::unordered_map<std::string, wtk::FunctionDeclare<Number_T>*>* func_map,
    wtk::index_t* max_ins,
    wtk::index_t* max_wit)
{
  *max_ins = 0;
  *max_wit = 0;

  for(size_t i = 0; i < switch_stmt->size(); i++)
  {
    switch(switch_stmt->caseBlock(i)->bodyType())
    {
    case wtk::CaseBlock<Number_T>::INVOKE:
    {
      auto finder = func_map->find(std::string(
          switch_stmt->caseBlock(i)->invokeBody()->name()));
      if(finder != func_map->end())
      {
        if(*max_ins < finder->second->instanceCount())
        {
          *max_ins = finder->second->instanceCount();
        }
        if(*max_wit < finder->second->shortWitnessCount())
        {
          *max_wit = finder->second->shortWitnessCount();
        }
      }
      else
      {
        log_error("function \'%s\' not found",
            switch_stmt->caseBlock(i)->invokeBody()->name());
        return false;
      }
      break;
    }
    case wtk::CaseBlock<Number_T>::ANONYMOUS:
    {
      if(*max_ins
          < switch_stmt->caseBlock(i)->anonymousBody()->instanceCount())
      {
        *max_ins = switch_stmt->caseBlock(i)->anonymousBody()->instanceCount();
      }
      if(*max_wit
          < switch_stmt->caseBlock(i)->anonymousBody()->shortWitnessCount())
      {
        *max_wit =
          switch_stmt->caseBlock(i)->anonymousBody()->shortWitnessCount();
      }
      break;
    }
    }
  }

  return true;
}

ALWAYS_INLINE inline bool listInsertSingleHelper(
    wtk::utils::SkipList<wtk::index_t>* const list, wtk::index_t const wire)
{
  if(!list->insert(wire))
  {
    log_error("Wire $%" PRIu64 " is already assigned.", wire);
    return false;
  }

  return true;
}

inline bool listOutputWires(
    wtk::WireList* const wires, wtk::utils::SkipList<wtk::index_t>* const list)
{
  for(size_t i = 0; i < wires->size(); i++)
  {
    switch(wires->type(i))
    {
    case wtk::WireList::SINGLE:
    {
      if(!list->insert(wires->single(i)))
      {
        log_error("Wire $%" PRIu64 " is already assigned.", wires->single(i));
        return false;
      }
      break;
    }
    case wtk::WireList::RANGE:
    {
      wtk::WireRange* range = wires->range(i);
      if(!list->insert(range->first(), range->last()))
      {
        log_error("Wire range $%" PRIu64 " ... $%" PRIu64
            " has elements which are already assigned.",
            range->first(), range->last());
        return false;
      }
      break;
    }
    }
  }

  return true;
}


template<typename Number_T>
bool listAssignedWires(
    wtk::DirectiveList<Number_T>* const directives,
    wtk::utils::SkipList<wtk::index_t>* const list)
{
  for(size_t i = 0; i < directives->size(); i++)
  {
    switch(directives->type(i))
    {
    case wtk::DirectiveList<Number_T>::BINARY_GATE:
    {
      if(!listInsertSingleHelper(
            list, directives->binaryGate(i)->outputWire()))
      {
        return false;
      }
      break;
    }
    case wtk::DirectiveList<Number_T>::BINARY_CONST_GATE:
    {
      if(!listInsertSingleHelper(
            list, directives->binaryConstGate(i)->outputWire()))
      {
        return false;
      }
      break;
    }
    case wtk::DirectiveList<Number_T>::UNARY_GATE:
    {
      if(!listInsertSingleHelper(list, directives->unaryGate(i)->outputWire()))
      {
        return false;
      }
      break;
    }
    case wtk::DirectiveList<Number_T>::INPUT:
    {
      if(!listInsertSingleHelper(list, directives->input(i)->outputWire()))
      {
        return false;
      }
      break;
    }
    case wtk::DirectiveList<Number_T>::ASSIGN:
    {
      if(!listInsertSingleHelper(list, directives->assign(i)->outputWire()))
      {
        return false;
      }
      break;
    }
    case wtk::DirectiveList<Number_T>::ASSERT_ZERO: { break; }
    case wtk::DirectiveList<Number_T>::DELETE_SINGLE: { break; }
    case wtk::DirectiveList<Number_T>::DELETE_RANGE: { break; }
    case wtk::DirectiveList<Number_T>::FUNCTION_INVOKE:
    {
      if(!listOutputWires(directives->functionInvoke(i)->outputList(), list))
      {
        return false;
      }
      break;
    }
    case wtk::DirectiveList<Number_T>::ANON_FUNCTION:
    {
      if(!listOutputWires(directives->anonFunction(i)->outputList(), list))
      {
        return false;
      }
      break;
    }
    case wtk::DirectiveList<Number_T>::FOR_LOOP:
    {
      if(!listOutputWires(directives->forLoop(i)->outputList(), list))
      {
        return false;
      }
      break;
    }
    case wtk::DirectiveList<Number_T>::SWITCH_STATEMENT:
    {
      if(!listOutputWires(directives->switchStatement(i)->outputList(), list))
      {
        return false;
      }
      break;
    }
    }
  }

  return true;
}

} } // namespace wtk::utils
