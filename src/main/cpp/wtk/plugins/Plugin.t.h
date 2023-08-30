/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace plugins {

// Search a std::tuple by type rather than index.
// source: https://stackoverflow.com/a/16594174
template<size_t Idx, typename Key_T, typename First_T, typename... Rest_Ts>
struct tuple_searcher
{
  typedef typename tuple_searcher<Idx + 1, Key_T, Rest_Ts...>::type type;
  static constexpr int index = Idx;
};

template<size_t Idx, typename Key_T, typename... Types_Ts>
struct tuple_searcher<Idx, Key_T, Key_T, Types_Ts...>
{
  typedef tuple_searcher type;
  static constexpr int index = Idx;
};


template<typename Number_T, typename... Wire_Ts>
template<typename Wire_T>
bool PluginsManager<Number_T, Wire_Ts...>::addPlugin(char const* const name,
    std::unique_ptr<Plugin<Number_T, Wire_T>>&& plugin)
{
  auto finder = this->plugins.find(name);
  if(finder == this->plugins.end())
  {
    finder = this->plugins.insert(finder,
        std::pair<char const* const, PluginBundle>(name, PluginBundle()));
  }

  size_t constexpr tpl_idx =
    tuple_searcher<0, Wire_T, Wire_Ts...>::type::index;

  if(std::get<tpl_idx>(finder->second.pluginBundle) != nullptr)
  {
    log_info("Duplicated plugin \"%s\"", name);
    return false;
  }

  std::get<tpl_idx>(finder->second.pluginBundle) = std::move(plugin);

  return true;
}

template<typename Number_T, typename... Wire_Ts>
template<typename Wire_T>
bool PluginsManager<Number_T, Wire_Ts...>::addBackend(wtk::type_idx type,
    wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  size_t constexpr tpl_idx =
    tuple_searcher<0, Wire_T, Wire_Ts...>::type::index;

  auto iter = this->plugins.begin();
  while(iter != this->plugins.end())
  {
    if(std::get<tpl_idx>(iter->second.pluginBundle) != nullptr)
    {
      if(!std::get<tpl_idx>(
            iter->second.pluginBundle)->addBackend(type, backend))
      {
        return false;
      }

      if(iter->second.reverseTypeIdxs.size() <= (size_t) type)
      {
        iter->second.reverseTypeIdxs.resize((size_t) type + 1);
      }

      // increment the tuple index, so that 0 can indicate an error
      iter->second.reverseTypeIdxs[(size_t) type] = tpl_idx + 1;
    }

    iter++;
  }

  return true;
}

template<typename Number_T, typename... Wire_Ts>
template<size_t I>
typename std::enable_if<I == sizeof...(Wire_Ts), Operation<Number_T>*>::type
PluginsManager<Number_T, Wire_Ts...>::createHelper(
    size_t tpl_idx, wtk::type_idx const type,
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding,
    std::tuple<std::unique_ptr<Plugin<Number_T, Wire_Ts>>...>& tpl)
{
  log_error("No suitable implementation for plugin \"%s\"",
      binding->name.c_str());

  (void) tpl_idx;
  (void) type;
  (void) signature;
  (void) binding;
  (void) tpl;
  return nullptr;
}

template<typename Number_T, typename... Wire_Ts>
template<size_t I>
typename std::enable_if<I < sizeof...(Wire_Ts), Operation<Number_T>*>::type
PluginsManager<Number_T, Wire_Ts...>::createHelper(
    size_t tpl_idx, wtk::type_idx const type,
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding,
    std::tuple<std::unique_ptr<Plugin<Number_T, Wire_Ts>>...>& tpl)
{
  if(tpl_idx == I)
  {
    return std::get<I>(tpl)->create(type, signature, binding);
  }
  else
  {
    return this->createHelper<I + 1>(tpl_idx, type, signature, binding, tpl);
  }
}

template<typename Number_T, typename... Wire_Ts>
Operation<Number_T>* PluginsManager<Number_T, Wire_Ts...>::create(
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  auto finder = this->plugins.find(binding->name.c_str());
  if(finder == this->plugins.end())
  {
    log_error("No such plugin \"%s\"", binding->name.c_str());
    return nullptr;
  }
  else
  {
    // find the "dominant type" of the function signature.
    size_t dom_type = (size_t) -1;
    size_t dom_wire = (size_t) -1;
    for(size_t i = 0; i < signature->inputs.size(); i++)
    {
      if(finder->second.reverseTypeIdxs.size()
          > (size_t) signature->inputs[i].type
          && finder->second.reverseTypeIdxs[signature->inputs[i].type] != 0)
      {
        dom_type = (size_t) signature->inputs[i].type;
        dom_wire =
          finder->second.reverseTypeIdxs[signature->inputs[i].type] - 1;
        break;
      }
    }

    if(dom_type == (size_t) -1)
    {
      // scan outputs for dom_type
      for(size_t i = 0; i < signature->outputs.size(); i++)
      {
        if(finder->second.reverseTypeIdxs.size()
            > (size_t) signature->outputs[i].type
            && finder->second.reverseTypeIdxs[signature->outputs[i].type] != 0)
        {
          dom_type = (size_t) signature->outputs[i].type;
          dom_wire =
            finder->second.reverseTypeIdxs[signature->outputs[i].type] - 1;
          break;
        }
      }
    }

    if(dom_type == (size_t) -1)
    {
      log_error("No suitable implementation for plugin \"%s\"",
          binding->name.c_str());
      return nullptr;
    }

    // Retrieve the plugin for dom_wire and pass along type/sig/binding
    return this->createHelper<0>(dom_wire, (wtk::type_idx) dom_type,
        signature, binding, finder->second.pluginBundle);
  }
}

} } // namespace wtk::plugins
