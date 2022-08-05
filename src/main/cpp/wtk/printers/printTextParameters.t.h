/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace printers {

template<typename Number_T>
void printTextHeaderParser(FILE* out, wtk::Parser<Number_T>* parser)
{
  printTextHeader(out, parser->characteristic, parser->degree,
      parser->version.major, parser->version.minor, parser->version.patch);
}

template<typename Number_T>
void printTextHeader(FILE* out, Number_T characteristic, size_t degree,
    size_t major, size_t minor, size_t patch)
{
  fprintf(out, "version %zu.%zu.%zu;\n", major, minor, patch);

  fprintf(out, "\nfield characteristic %s degree %zu;\n",
      wtk::utils::dec(characteristic).c_str(), degree);
}

template<typename Number_T>
void printTextResourceParser(FILE* out, wtk::Parser<Number_T>* parser)
{
  printTextResource(out, parser->resource);
}

void printTextResource(FILE* out, wtk::Resource resource)
{
  switch(resource)
  {
  case wtk::Resource::relation:
    fprintf(out, "relation ");
    break;
  case wtk::Resource::instance:
    fprintf(out, "instance ");
    break;
  case wtk::Resource::shortWitness:
    fprintf(out, "short_witness ");
    break;
  case wtk::Resource::invalid:
    fprintf(out, "invalid ");
    break;
  }
}

template<typename Number_T>
void printTextGateSetParser(FILE* out, wtk::Parser<Number_T>* parser)
{
  printTextGateSet(out, parser->gateSet);
}

void printTextGateSet(FILE* out, wtk::GateSet gateset)
{
  if(gateset.cannonical())
  {
    if(gateset.gateSet == GateSet::arithmetic)
    {
      fprintf(out, "gate_set : arithmetic;\n");
    }
    else
    {
      fprintf(out, "gate_set: boolean;\n");
    }
  }
  else if(gateset.gateSet == GateSet::arithmetic)
  {
    fprintf(out, "gate_set: ");
    char const* comma = "";

    if(gateset.enableAdd)
    {
      fprintf(out, "@add");
      comma = ", ";
    }
    if(gateset.enableAddC)
    {
      fprintf(out, "%s@addc", comma);
      comma = ", ";
    }
    if(gateset.enableMul)
    {
      fprintf(out, "%s@mul", comma);
      comma = ", ";
    }
    if(gateset.enableMulC)
    {
      fprintf(out, "%s@mulc", comma);
    }

    fprintf(out, ";\n");
  }
  else if(gateset.gateSet == GateSet::boolean)
  {
    fprintf(out, "gate_set: ");
    char const* comma = "";

    if(gateset.enableXor)
    {
      fprintf(out, "@xor");
      comma = ", ";
    }
    if(gateset.enableAnd)
    {
      fprintf(out, "%s@and", comma);
      comma = ", ";
    }
    if(gateset.enableNot)
    {
      fprintf(out, "%s@not", comma);
    }

    fprintf(out, ";\n");
  }
  else
  {
    fprintf(out, "gate_set: invalid;\n");
  }
}

template<typename Number_T>
void printTextFeatureTogglesParser(FILE* out, wtk::Parser<Number_T>* parser)
{
  printTextFeatureToggles(out, parser->featureToggles);
}

void printTextFeatureToggles(FILE* out, wtk::FeatureToggles toggles)
{
  fprintf(out, "features: ");
  if(toggles.complete())
  {
    fprintf(out, "@function, @for, @switch;\n");
  }
  else if(toggles.simple())
  {
    fprintf(out, "simple;\n");
  }
  else
  {
    char const* comma = "";

    if(toggles.functionToggle)
    {
      fprintf(out, "@function");
      comma = ", ";
    }
    if(toggles.forLoopToggle)
    {
      fprintf(out, "%s@for", comma);
      comma = ", ";
    }
    if(toggles.switchCaseToggle)
    {
      fprintf(out, "%s@switch", comma);
    }

    fprintf(out, ";\n");
  }
}

} } // namespace wtk::printers
