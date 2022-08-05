/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace viz {

template<typename Number_T>
TreeVisualizer<Number_T>::TreeVisualizer(
    FILE* out, std::string fg, std::string bg)
  : outFile(out), foreground(fg), background(bg),
    useFg(fg != ""), useBg(bg != "") { }

template<typename Number_T>
TreeVisualizer<Number_T>::TreeVisualizer(
    FILE* out, char const* const fg, char const* const bg)
  : outFile(out), foreground(fg), background(bg),
    useFg(this->foreground != ""), useBg(this->background != "") { }

template<typename Number_T>
void TreeVisualizer<Number_T>::printBinaryGate(BinaryGate* gate)
{
  switch(gate->calculation())
  {
  case BinaryGate::ADD:
  {
    if(this->useFg)
    {
      fprintf(this->outFile,
          "%ss%zuw%" PRIu64 " [label=\"$%" PRIu64 "<- @add\", "
          "color=\"%s\", fontcolor=\"%s\"];\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 " [color=\"%s\"];\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 " [color=\"%s\"];\n",
          this->indent.get(), this->curScope, gate->outputWire(),
          gate->outputWire(), this->foreground.c_str(),
          this->foreground.c_str(), this->indent.get(), this->curScope,
          gate->leftWire(), this->curScope, gate->outputWire(),
          this->foreground.c_str(), this->indent.get(), this->curScope,
          gate->rightWire(), this->curScope, gate->outputWire(),
          this->foreground.c_str());
    }
    else
    {
      fprintf(this->outFile,
          "%ss%zuw%" PRIu64 " [label=\"$%" PRIu64 "<- @add\"];\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 ";\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 ";\n",
          this->indent.get(), this->curScope, gate->outputWire(),
          gate->outputWire(), this->indent.get(), this->curScope,
          gate->leftWire(), this->curScope, gate->outputWire(),
          this->indent.get(), this->curScope, gate->rightWire(),
          this->curScope, gate->outputWire());
    }
    break;
  }
  case BinaryGate::MUL:
  {
    if(this->useFg)
    {
      fprintf(this->outFile,
          "%ss%zuw%" PRIu64 " [label=\"$%" PRIu64 "<- @mul\", "
          "color=\"%s\", fontcolor=\"%s\"];\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 " [color=\"%s\"];\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 " [color=\"%s\"];\n",
          this->indent.get(), this->curScope, gate->outputWire(),
          gate->outputWire(), this->foreground.c_str(),
          this->foreground.c_str(), this->indent.get(), this->curScope,
          gate->leftWire(), this->curScope, gate->outputWire(),
          this->foreground.c_str(), this->indent.get(), this->curScope,
          gate->rightWire(), this->curScope, gate->outputWire(),
          this->foreground.c_str());
    }
    else
    {
      fprintf(this->outFile,
          "%ss%zuw%" PRIu64 " [label=\"$%" PRIu64 "<- @mul\"];\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 ";\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 ";\n",
          this->indent.get(), this->curScope, gate->outputWire(),
          gate->outputWire(), this->indent.get(), this->curScope,
          gate->leftWire(), this->curScope, gate->outputWire(),
          this->indent.get(), this->curScope, gate->rightWire(),
          this->curScope, gate->outputWire());
    }
    break;
  }
  case BinaryGate::AND:
  {
    if(this->useFg)
    {
      fprintf(this->outFile,
          "%ss%zuw%" PRIu64 " [label=\"$%" PRIu64 "<- @and\", "
          "color=\"%s\", fontcolor=\"%s\"];\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 " [color=\"%s\"];\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 " [color=\"%s\"];\n",
          this->indent.get(), this->curScope, gate->outputWire(),
          gate->outputWire(), this->foreground.c_str(),
          this->foreground.c_str(), this->indent.get(), this->curScope,
          gate->leftWire(), this->curScope, gate->outputWire(),
          this->foreground.c_str(), this->indent.get(), this->curScope,
          gate->rightWire(), this->curScope, gate->outputWire(),
          this->foreground.c_str());
    }
    else
    {
      fprintf(this->outFile,
          "%ss%zuw%" PRIu64 " [label=\"$%" PRIu64 "<- @and\"];\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 ";\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 ";\n",
          this->indent.get(), this->curScope, gate->outputWire(),
          gate->outputWire(), this->indent.get(), this->curScope,
          gate->leftWire(), this->curScope, gate->outputWire(),
          this->indent.get(), this->curScope, gate->rightWire(),
          this->curScope, gate->outputWire());
    }
    break;
  }
  case BinaryGate::XOR:
  {
    if(this->useFg)
    {
      fprintf(this->outFile,
          "%ss%zuw%" PRIu64 " [label=\"$%" PRIu64 "<- @xor\", "
          "color=\"%s\", fontcolor=\"%s\"];\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 " [color=\"%s\"];\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 " [color=\"%s\"];\n",
          this->indent.get(), this->curScope, gate->outputWire(),
          gate->outputWire(), this->foreground.c_str(),
          this->foreground.c_str(), this->indent.get(), this->curScope,
          gate->leftWire(), this->curScope, gate->outputWire(),
          this->foreground.c_str(), this->indent.get(), this->curScope,
          gate->rightWire(), this->curScope, gate->outputWire(),
          this->foreground.c_str());
    }
    else
    {
      fprintf(this->outFile,
          "%ss%zuw%" PRIu64 " [label=\"$%" PRIu64 "<- @xor\"];\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 ";\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 ";\n",
          this->indent.get(), this->curScope, gate->outputWire(),
          gate->outputWire(), this->indent.get(), this->curScope,
          gate->leftWire(), this->curScope, gate->outputWire(),
          this->indent.get(), this->curScope, gate->rightWire(),
          this->curScope, gate->outputWire());
    }
    break;
  }
  }
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printBinaryConstGate(
    BinaryConstGate<Number_T>* gate)
{
  switch(gate->calculation())
  {
  case BinaryConstGate<Number_T>::ADDC:
  {
    if(this->useFg)
    {
      fprintf(this->outFile,
          "%ss%zuc%" PRIu64 " [label=\"< %s >\", "
          "color=\"%s\", fontcolor=\"%s\"];\n"
          "%ss%zuw%" PRIu64 " [label=\"$%" PRIu64 " <- @addc\", "
          "color=\"%s\", fontcolor=\"%s\"];\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 " [color=\"%s\"];\n"
          "%ss%zuc%" PRIu64 " -> s%zuw%" PRIu64 " [color=\"%s\"];\n",
          this->indent.get(), this->curScope, gate->outputWire(),
          wtk::utils::dec(gate->rightValue()).c_str(),
          this->foreground.c_str(), this->foreground.c_str(),
          this->indent.get(), this->curScope, gate->outputWire(),
          gate->outputWire(), this->foreground.c_str(),
          this->foreground.c_str(), this->indent.get(), this->curScope,
          gate->leftWire(), this->curScope, gate->outputWire(),
          this->foreground.c_str(), this->indent.get(), this->curScope,
          gate->outputWire(), this->curScope, gate->outputWire(),
          this->foreground.c_str());
    }
    else
    {
      fprintf(this->outFile,
          "%ss%zuc%" PRIu64 " [label=\"< %s >\"];\n"
          "%ss%zuw%" PRIu64 " [label=\"$%" PRIu64 " <- @addc\"];\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 ";\n"
          "%ss%zuc%" PRIu64 " -> s%zuw%" PRIu64 ";\n",
          this->indent.get(), this->curScope, gate->outputWire(),
          wtk::utils::dec(gate->rightValue()).c_str(),
          this->indent.get(), this->curScope, gate->outputWire(),
          gate->outputWire(), this->indent.get(), this->curScope,
          gate->leftWire(), this->curScope, gate->outputWire(),
          this->indent.get(), this->curScope, gate->outputWire(),
          this->curScope, gate->outputWire());
    }
    break;
  }
  case BinaryConstGate<Number_T>::MULC:
  {
    if(this->useFg)
    {
      fprintf(this->outFile,
          "%ss%zuc%" PRIu64 " [label=\"< %s >\", "
          "color=\"%s\", fontcolor=\"%s\"];\n"
          "%ss%zuw%" PRIu64 " [label=\"$%" PRIu64 " <- @mulc\", "
          "color=\"%s\", fontcolor=\"%s\"];\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 " [color=\"%s\"];\n"
          "%ss%zuc%" PRIu64 " -> s%zuw%" PRIu64 " [color=\"%s\"];\n",
          this->indent.get(), this->curScope, gate->outputWire(),
          wtk::utils::dec(gate->rightValue()).c_str(),
          this->foreground.c_str(), this->foreground.c_str(),
          this->indent.get(), this->curScope, gate->outputWire(),
          gate->outputWire(), this->foreground.c_str(),
          this->foreground.c_str(), this->indent.get(), this->curScope,
          gate->leftWire(), this->curScope, gate->outputWire(),
          this->foreground.c_str(), this->indent.get(), this->curScope,
          gate->outputWire(), this->curScope, gate->outputWire(),
          this->foreground.c_str());
    }
    else
    {
      fprintf(this->outFile,
          "%ss%zuc%" PRIu64 " [label=\"< %s >\"];\n"
          "%ss%zuw%" PRIu64 " [label=\"$%" PRIu64 " <- @mulc\"];\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 ";\n"
          "%ss%zuc%" PRIu64 " -> s%zuw%" PRIu64 ";\n",
          this->indent.get(), this->curScope, gate->outputWire(),
          wtk::utils::dec(gate->rightValue()).c_str(),
          this->indent.get(), this->curScope, gate->outputWire(),
          gate->outputWire(), this->indent.get(), this->curScope,
          gate->leftWire(), this->curScope, gate->outputWire(),
          this->indent.get(), this->curScope, gate->outputWire(),
          this->curScope, gate->outputWire());
    }
    break;
  }
  }
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printUnaryGate(UnaryGate* gate)
{
  switch(gate->calculation())
  {
  case UnaryGate::NOT:
  {
    if(this->useFg)
    {
      fprintf(this->outFile,
          "%ss%zuw%" PRIu64 "[label=\"$%" PRIu64 "<- @not\", "
          "color=\"%s\", fontcolor=\"%s\"];\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 " [color=\"%s\"];\n",
          this->indent.get(), this->curScope, gate->outputWire(),
          gate->outputWire(), this->foreground.c_str(),
          this->foreground.c_str(),this->indent.get(), this->curScope,
          gate->inputWire(), this->curScope, gate->outputWire(),
          this->foreground.c_str());
    }
    else
    {
      fprintf(this->outFile,
          "%ss%zuw%" PRIu64 "[label=\"$%" PRIu64 "<- @not\"];\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 ";\n",
          this->indent.get(), this->curScope, gate->outputWire(),
          gate->outputWire(), this->indent.get(), this->curScope,
          gate->inputWire(), this->curScope, gate->outputWire());
    }
    break;
  }
  case UnaryGate::COPY:
  {
    if(this->useFg)
    {
      fprintf(this->outFile,
          "%ss%zuw%" PRIu64 "[label=\"$%" PRIu64 "<- COPY\", "
          "color=\"%s\", fontcolor=\"%s\"];\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 " [color=\"%s\"];\n",
          this->indent.get(), this->curScope, gate->outputWire(),
          gate->outputWire(), this->foreground.c_str(),
          this->foreground.c_str(),this->indent.get(), this->curScope,
          gate->inputWire(), this->curScope, gate->outputWire(),
          this->foreground.c_str());
    }
    else
    {
      fprintf(this->outFile,
          "%ss%zuw%" PRIu64 "[label=\"$%" PRIu64 "<- @not\"];\n"
          "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 ";\n",
          this->indent.get(), this->curScope, gate->outputWire(),
          gate->outputWire(), this->indent.get(), this->curScope,
          gate->inputWire(), this->curScope, gate->outputWire());
    }
    break;
  }
  }
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printInput(Input* input)
{
  switch(input->stream())
  {
  case Input::INSTANCE:
  {
    if(this->useFg)
    {
      fprintf(this->outFile,
          "%ss%zuw%" PRIu64 "[label=\"$%" PRIu64 " <- @instance\", "
          "color=\"%s\", fontcolor=\"%s\"];\n",
          this->indent.get(), this->curScope, input->outputWire(),
          input->outputWire(), this->foreground.c_str(),
          this->foreground.c_str());
    }
    else
    {
      fprintf(this->outFile,
          "%ss%zuw%" PRIu64 "[label=\"$%" PRIu64 " <- @instance\"];\n",
          this->indent.get(), this->curScope, input->outputWire(),
          input->outputWire());
    }
    break;
  }
  case Input::SHORT_WITNESS:
  {
    if(this->useFg)
    {
      fprintf(this->outFile,
          "%ss%zuw%" PRIu64 "[label=\"$%" PRIu64 " <- @short_witness\", "
          "color=\"%s\", fontcolor=\"%s\"];\n",
          this->indent.get(), this->curScope, input->outputWire(),
          input->outputWire(), this->foreground.c_str(),
          this->foreground.c_str());
    }
    else
    {
      fprintf(this->outFile,
          "%ss%zuw%" PRIu64 "[label=\"$%" PRIu64 " <- @short_witness\"];\n",
          this->indent.get(), this->curScope, input->outputWire(),
          input->outputWire());
    }
    break;
  }
  }
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printAssign(Assign<Number_T>* assign)
{
  if(this->useFg)
  {
    fprintf(this->outFile,
        "%ss%zuc%" PRIu64 " [label=\"< %s >\", "
        "color=\"%s\", fontcolor=\"%s\"];\n"
        "%ss%zuw%" PRIu64 " [label=\"$%" PRIu64 " <- ASSIGN\", "
        "color=\"%s\", fontcolor=\"%s\"];\n"
        "%ss%zuc%" PRIu64 " -> s%zuw%" PRIu64 " [color=\"%s\"];\n",
        this->indent.get(), this->curScope, assign->outputWire(),
        wtk::utils::dec(assign->constValue()).c_str(),
        this->foreground.c_str(), this->foreground.c_str(),
        this->indent.get(), this->curScope, assign->outputWire(),
        assign->outputWire(), this->foreground.c_str(),
        this->foreground.c_str(), this->indent.get(), this->curScope,
        assign->outputWire(), this->curScope, assign->outputWire(),
        this->foreground.c_str());
  }
  else
  {
    fprintf(this->outFile,
        "%ss%zuc%" PRIu64 " [label=\"< %s >\"];\n"
        "%ss%zuw%" PRIu64 " [label=\"$%" PRIu64 " <- ASSIGN\"];\n"
        "%ss%zuc%" PRIu64 " -> s%zuw%" PRIu64 ";\n",
        this->indent.get(), this->curScope, assign->outputWire(),
        wtk::utils::dec(assign->constValue()).c_str(),
        this->indent.get(), this->curScope, assign->outputWire(),
        assign->outputWire(), this->indent.get(), this->curScope,
        assign->outputWire(), this->curScope, assign->outputWire());
  }
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printAssertZero(Terminal* assertZero)
{
  if(this->useFg)
  {
    fprintf(this->outFile,
        "%ss%zuaz%zu [label=\"@assert_zero($%" PRIu64 ")\", "
        "color=\"%s\", fontcolor=\"%s\"];\n"
        "%ss%zuw%" PRIu64 " -> s%zuaz%zu [color=\"%s\"];\n",
        this->indent.get(), this->curScope, this->assertZeroNum,
        assertZero->wire(), this->foreground.c_str(), this->foreground.c_str(),
        this->indent.get(), this->curScope, assertZero->wire(),
        this->curScope, this->assertZeroNum, this->foreground.c_str());
  }
  else
  {
    fprintf(this->outFile,
        "%ss%zuaz%zu [label=\"@assert_zero($%" PRIu64 ")\"];\n"
        "%ss%zuw%" PRIu64 " -> s%zuaz%zu;\n",
        this->indent.get(), this->curScope, this->assertZeroNum,
        assertZero->wire(), this->indent.get(), this->curScope,
        assertZero->wire(), this->curScope, this->assertZeroNum);
  }
  this->assertZeroNum++;
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printDeleteSingle(Terminal* del)
{
  (void) del;
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printDeleteRange(WireRange* del)
{
  (void) del;
}

template<typename Number_T>
size_t TreeVisualizer<Number_T>::countWireList(WireList* list)
{
  size_t count = 0;

  for(size_t i = 0; i < list->size(); i++)
  {
    switch(list->type(i))
    {
    case WireList::SINGLE:
    {
      count++;
      break;
    }
    case WireList::RANGE:
    {
      count += (size_t) (list->range(i)->last() - list->range(i)->first()) + 1;
      break;
    }
    }
  }

  return count;
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printInputListEdges(
    WireList* list, size_t const outer_scope, size_t const output_count)
{
  wtk::index_t wire_num = (wtk::index_t) output_count;

  for(size_t i = 0; i < list->size(); i++)
  {
    switch(list->type(i))
    {
    case WireList::SINGLE:
    {
      if(this->useFg)
      {
        fprintf(this->outFile,
            "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 " [color=\"%s\"];\n",
            this->indent.get(), outer_scope, list->single(i),
            this->curScope, wire_num, this->foreground.c_str());
      }
      else
      {
        fprintf(this->outFile, "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 ";\n",
            this->indent.get(), outer_scope, list->single(i),
            this->curScope, wire_num);
      }
      wire_num++;
      break;
    }
    case WireList::RANGE:
    {
      WireRange* range = list->range(i);
      for(wtk::index_t j = range->first(); j <= range->last(); j++)
      {
        if(this->useFg)
        {
          fprintf(this->outFile,
              "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 " [color=\"%s\"];\n",
              this->indent.get(), outer_scope, j, this->curScope, wire_num,
              this->foreground.c_str());
        }
        else
        {
          fprintf(this->outFile, "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 ";\n",
              this->indent.get(), outer_scope, j, this->curScope, wire_num);
        }
        wire_num++;
      }
      break;
    }
    }
  }
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printInputListNodes(
    WireList* list, size_t const output_count)
{
  wtk::index_t wire_num = (wtk::index_t) output_count;

  for(size_t i = 0; i < list->size(); i++)
  {
    switch(list->type(i))
    {
    case WireList::SINGLE:
    {
      if(this->useFg)
      {
        fprintf(this->outFile,
            "%ss%zuw%" PRIu64 "[label=\"in: $%" PRIu64 " -> $%" PRIu64 "\", "
            "color=\"%s\", fontcolor=\"%s\"];\n",
            this->indent.get(), this->curScope, wire_num, list->single(i),
            wire_num, this->foreground.c_str(), this->foreground.c_str());
      }
      else
      {
        fprintf(this->outFile, "%ss%zuw%" PRIu64 "[label=\"in: $%" PRIu64
            " -> $%" PRIu64 "\"];\n", this->indent.get(), this->curScope,
            wire_num, list->single(i), wire_num);
      }
      wire_num++;
      break;
    }
    case WireList::RANGE:
    {
      WireRange* range = list->range(i);
      for(wtk::index_t j = range->first(); j <= range->last(); j++)
      {
        if(this->useFg)
        {
          fprintf(this->outFile, "%ss%zuw%" PRIu64 "[label=\"in: $%" PRIu64
              " -> $%" PRIu64 "\", color=\"%s\", fontcolor=\"%s\"];\n",
              this->indent.get(), this->curScope, wire_num, j, wire_num,
              this->foreground.c_str(), this->foreground.c_str());
        }
        else
        {
          fprintf(this->outFile, "%ss%zuw%" PRIu64 "[label=\"in: $%" PRIu64
              " -> $%" PRIu64 "\"];\n", this->indent.get(), this->curScope,
              wire_num, j, wire_num);
        }
        wire_num++;
      }
      break;
    }
    }
  }
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printOutputList(
    WireList* list, size_t const outer_scope)
{
  wtk::index_t wire_num = 0;

  for(size_t i = 0; i < list->size(); i++)
  {
    switch(list->type(i))
    {
    case WireList::SINGLE:
    {
      if(this->useFg)
      {
        fprintf(this->outFile,
            "%ss%zuw%" PRIu64 "[label=\"out: $%" PRIu64 "\", "
            "color=\"%s\", fontcolor=\"%s\"];\n"
            "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 " [color=\"%s\"];\n",
            this->indent.get(), outer_scope, list->single(i), list->single(i),
            this->foreground.c_str(), this->foreground.c_str(),
            this->indent.get(), this->curScope, wire_num, outer_scope,
            list->single(i), this->foreground.c_str());
      }
      else
      {
        fprintf(this->outFile,
            "%ss%zuw%" PRIu64 "[label=\"out: $%" PRIu64 "\"];\n"
            "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 ";\n",
            this->indent.get(), outer_scope, list->single(i), list->single(i),
            this->indent.get(), this->curScope, wire_num, outer_scope,
            list->single(i));
      }
      wire_num++;
      break;
    }
    case WireList::RANGE:
    {
      WireRange* range = list->range(i);
      for(wtk::index_t j = range->first(); j <= range->last(); j++)
      {
        if(this->useFg)
        {
          fprintf(this->outFile,
              "%ss%zuw%" PRIu64 "[label=\"out: $%" PRIu64 "\", "
              "color=\"%s\", fontcolor=\"%s\"];\n"
              "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 " [color=\"%s\"];\n",
              this->indent.get(), outer_scope, j, j, this->foreground.c_str(),
              this->foreground.c_str(), this->indent.get(), this->curScope,
              wire_num, outer_scope, j, this->foreground.c_str());
        }
        else
        {
          fprintf(this->outFile,
              "%ss%zuw%" PRIu64 "[label=\"out: $%" PRIu64 "\"];\n"
              "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 ";\n",
              this->indent.get(), outer_scope, j, j, this->indent.get(),
              this->curScope, wire_num, outer_scope, j);
        }
        wire_num++;
      }
      break;
    }
    }
  }
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printFunctionInvoke(FunctionInvoke* invoke)
{
  size_t const outer_scope = this->curScope;
  this->curScope = ++this->scopeNum;

  size_t const output_count = this->countWireList(invoke->outputList());
  size_t const input_count = this->countWireList(invoke->inputList());

  this->printInputListEdges(invoke->inputList(), outer_scope, output_count);

  fprintf(this->outFile,
      "%ssubgraph cluster_s%zu {\n  %slabel=\"@call(%s)\";\n",
      this->indent.get(), this->curScope, this->indent.get(), invoke->name());
  this->indent.inc();

  if(this->useFg)
  {
    fprintf(this->outFile, "%scolor=\"%s\";\n%sfontcolor=\"%s\";\n",
        this->indent.get(), this->foreground.c_str(),
        this->indent.get(), this->foreground.c_str());
  }

  this->printInputListNodes(invoke->inputList(), output_count);

  if(this->useFg)
  {
    fprintf(this->outFile, "%scall%zu [label=\"@call(%s)\", "
        "color=\"%s\", fontcolor=\"%s\"];\n",
        this->indent.get(), this->curScope, invoke->name(),
        this->foreground.c_str(), this->foreground.c_str());
  }
  else
  {
    fprintf(this->outFile, "%scall%zu [label=\"@call(%s)\"];\n",
        this->indent.get(), this->curScope, invoke->name());
  }

  for(wtk::index_t i = (wtk::index_t) output_count; i <= (wtk::index_t) input_count; i++)
  {
    if(this->useFg)
    {
      fprintf(this->outFile, "%ss%zuw%" PRIu64 " -> call%zu [color=\"%s\"];\n",
          this->indent.get(), this->curScope, i, this->curScope,
          this->foreground.c_str());
    }
    else
    {
      fprintf(this->outFile, "%ss%zuw%" PRIu64 " -> call%zu;\n",
          this->indent.get(), this->curScope, i, this->curScope);
    }
  }

  for(wtk::index_t i = 0; i < (wtk::index_t) output_count; i++)
  {
    if(this->useFg)
    {
      fprintf(this->outFile,
          "%ss%zuw%" PRIu64 " [label=\"out -> $%" PRIu64 "\", "
          "color=\"%s\", fontcolor=\"%s\"];\n"
          "%scall%zu -> s%zuw%" PRIu64 " [color=\"%s\"];\n",
          this->indent.get(), this->curScope, i, i, this->foreground.c_str(),
          this->foreground.c_str(), this->indent.get(), this->curScope,
          this->curScope, i, this->foreground.c_str());
    }
    else
    {
      fprintf(this->outFile,
          "%ss%zuw%" PRIu64 " [label=\"out -> $%" PRIu64 "\"];\n"
          "%scall%zu -> s%zuw%" PRIu64 ";\n",
          this->indent.get(), this->curScope, i, i,
          this->indent.get(), this->curScope, this->curScope, i);
    }
  }

  this->indent.dec();
  fprintf(this->outFile, "%s}\n", this->indent.get());

  this->printOutputList(invoke->outputList(), outer_scope);

  this->curScope = outer_scope;
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printAnonFunction(AnonFunction<Number_T>* anon)
{
  size_t const outer_scope = this->curScope;
  this->curScope = ++this->scopeNum;

  size_t const output_count = this->countWireList(anon->outputList());

  this->printInputListEdges(anon->inputList(), outer_scope, output_count);

  fprintf(this->outFile,
      "%ssubgraph cluster_s%zu {\n  %slabel=\"@anon_call\";\n",
      this->indent.get(), this->curScope, this->indent.get());
  this->indent.inc();

  if(this->useFg)
  {
    fprintf(this->outFile, "%scolor=\"%s\";\n%sfontcolor=\"%s\";\n",
        this->indent.get(), this->foreground.c_str(),
        this->indent.get(), this->foreground.c_str());
  }

  this->printInputListNodes(anon->inputList(), output_count);

  this->printDirectiveList(anon->body());

  this->indent.dec();
  fprintf(this->outFile, "%s}\n", this->indent.get());

  this->printOutputList(anon->outputList(), outer_scope);

  this->curScope = outer_scope;
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printIterExpr(IterExpr* expr)
{
  switch(expr->type())
  {
  case IterExpr::LITERAL:
  {
    fprintf(this->outFile, "%" PRIu64, expr->literal());
    break;
  }
  case IterExpr::ITERATOR:
  {
    fputs(expr->name(), this->outFile);
    break;
  }
  case IterExpr::ADD:
  {
    fputs("(", this->outFile);
    this->printIterExpr(expr->lhs());
    fputs(" + ", this->outFile);
    this->printIterExpr(expr->rhs());
    fputs(")", this->outFile);
    break;
  }
  case IterExpr::SUB:
  {
    fputs("(", this->outFile);
    this->printIterExpr(expr->lhs());
    fputs(" - ", this->outFile);
    this->printIterExpr(expr->rhs());
    fputs(")", this->outFile);
    break;
  }
  case IterExpr::MUL:
  {
    fputs("(", this->outFile);
    this->printIterExpr(expr->lhs());
    fputs(" * ", this->outFile);
    this->printIterExpr(expr->rhs());
    fputs(")", this->outFile);
    break;
  }
  case IterExpr::DIV:
  {
    fputs("(", this->outFile);
    this->printIterExpr(expr->lhs());
    fprintf(this->outFile, " / %" PRIu64, expr->literal());
    break;
  }
  }
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printLoopInputMappingNodes(
    IterExprWireList* list, size_t const loop_scope)
{
  for(size_t i = 0; i < list->size(); i++)
  {
    switch(list->type(i))
    {
    case IterExprWireList::SINGLE:
    {
      fprintf(this->outFile, "%ss%zumi%zu [label=\"map in:\\n$",
          this->indent.get(), loop_scope, i);
      this->printIterExpr(list->single(i));
      if(this->useFg)
      {
        fprintf(this->outFile, "\", color=\"%s\", fontcolor=\"%s\"];\n",
            this->foreground.c_str(), this->foreground.c_str());
      }
      else
      {
        fputs("\"];\n", this->outFile);
      }
      break;
    }
    case IterExprWireList::RANGE:
    {
      fprintf(this->outFile, "%ss%zumi%zu [label=\"map in:\\n$",
          this->indent.get(), loop_scope, i);
      this->printIterExpr(list->range(i)->first());
      fputs(" ... $", this->outFile);
      this->printIterExpr(list->range(i)->last());
      if(this->useFg)
      {
        fprintf(this->outFile, "\", color=\"%s\", fontcolor=\"%s\"];\n",
            this->foreground.c_str(), this->foreground.c_str());
      }
      else
      {
        fputs("\"];\n", this->outFile);
      }
      break;
    }
    }
  }
}

template<typename Number_T>
size_t TreeVisualizer<Number_T>::printLoopOutputMappingNodes(
    IterExprWireList* list, size_t const loop_scope)
{
  std::unordered_map<std::string, wtk::index_t> iters_map;
  for(size_t i = 0; i < this->loopBounds.size(); i++)
  {
    std::string n(this->loopBounds[i].name);
    iters_map[n] = this->loopBounds[i].first;
  }

  size_t outputs_count = 0;
  for(size_t i = 0; i < list->size(); i++)
  {
    switch(list->type(i))
    {
    case IterExprWireList::SINGLE:
    {
      fprintf(this->outFile, "%ss%zumo%zu [label=\"map out:\\n$",
          this->indent.get(), loop_scope, i);
      this->printIterExpr(list->single(i));
      if(this->useFg)
      {
        fprintf(this->outFile, "\", color=\"%s\", fontcolor=\"%s\"];\n",
            this->foreground.c_str(), this->foreground.c_str());
      }
      else
      {
        fputs("\"];\n", this->outFile);
      }

      outputs_count++;
      break;
    }
    case IterExprWireList::RANGE:
    {
      fprintf(this->outFile, "%ss%zumo%zu [label=\"map out:\\n$",
          this->indent.get(), loop_scope, i);
      this->printIterExpr(list->range(i)->first());
      fputs(" ... $", this->outFile);
      this->printIterExpr(list->range(i)->last());
      if(this->useFg)
      {
        fprintf(this->outFile, "\", color=\"%s\", fontcolor=\"%s\"];\n",
            this->foreground.c_str(), this->foreground.c_str());
      }
      else
      {
        fputs("\"];\n", this->outFile);
      }

      outputs_count = outputs_count
        + (size_t) (wtk::utils::iterExprEval(list->range(i)->last(), iters_map)
            - wtk::utils::iterExprEval(list->range(i)->first(), iters_map))
        + 1;
      break;
    }
    }
  }

  return outputs_count;
}

template<typename Number_T>
size_t TreeVisualizer<Number_T>::printLoopInputNodes(
    IterExprWireList* list, size_t const output_count)
{
  std::unordered_map<std::string, wtk::index_t> iters_map;
  for(size_t i = 0; i < this->loopBounds.size(); i++)
  {
    std::string n(this->loopBounds[i].name);
    iters_map[n] = this->loopBounds[i].first;
  }

  wtk::index_t wire_num = output_count;
  for(size_t i = 0; i < list->size(); i++)
  {
    switch(list->type(i))
    {
    case IterExprWireList::SINGLE:
    {
      if(this->useFg)
      {
        fprintf(this->outFile,
            "%ss%zuw%" PRIu64 " [label=\"in: $%" PRIu64 "\", "
            "color=\"%s\", fontcolor=\"%s\"];\n",
            this->indent.get(), this->curScope, wire_num, wire_num,
            this->foreground.c_str(), this->foreground.c_str());
      }
      else
      {
        fprintf(this->outFile,
            "%ss%zuw%" PRIu64 " [label=\"in: $%" PRIu64 "\"];\n",
            this->indent.get(), this->curScope, wire_num, wire_num);
      }
        wire_num++;
      break;
    }
    case IterExprWireList::RANGE:
    {
      IterExprWireRange* range = list->range(i);
      wtk::index_t const first =
        wtk::utils::iterExprEval(range->first(), iters_map);
      wtk::index_t const last =
        wtk::utils::iterExprEval(range->last(), iters_map);

      for(wtk::index_t j = first; j <= last; j++)
      {
        if(this->useFg)
        {
          fprintf(this->outFile,
              "%ss%zuw%" PRIu64 " [label=\"in: $%" PRIu64 "\", "
              "color=\"%s\", fontcolor=\"%s\"];\n",
              this->indent.get(), this->curScope, wire_num, wire_num,
              this->foreground.c_str(), this->foreground.c_str());
        }
        else
        {
          fprintf(this->outFile,
              "%ss%zuw%" PRIu64 " [label=\"in: $%" PRIu64 "\"];\n",
              this->indent.get(), this->curScope, wire_num, wire_num);
        }
        wire_num++;
      }
      break;
    }
    }
  }

  return (size_t) (wire_num - output_count);
}

inline void printLoopInputEdgesHelper(
    IterExprWireList* list, size_t const output_count,
    size_t const loop_scope, size_t const cur_scope,
    std::unordered_map<std::string, wtk::index_t>& iters_map,
    std::unordered_set<std::string>& edges,
    std::vector<LoopBound>& bounds, size_t const bounds_idx)
{
  if(bounds_idx < bounds.size())
  {
    LoopBound* bound = &bounds[bounds_idx];
    std::string n(bound->name);
    for(wtk::index_t i = bound->first; i <= bound->last; i++)
    {
      iters_map[n] = i;
      printLoopInputEdgesHelper( list, output_count, loop_scope, cur_scope,
          iters_map, edges, bounds, bounds_idx + 1);
    }
  }
  else
  {
    wtk::index_t wire_num = output_count;
    for(size_t i = 0; i < list->size(); i++)
    {
      switch(list->type(i))
      {
      case IterExprWireList::SINGLE:
      {
        std::string edge("s");
        edge += wtk::utils::dec(loop_scope);
        edge += "mi";
        edge += wtk::utils::dec(i);
        edge += " -> ";
        edge += "s";
        edge += wtk::utils::dec(cur_scope);
        edge += "w";
        edge += wtk::utils::dec(wire_num);
        edges.insert(edge);
        wire_num++;
        break;
      }
      case IterExprWireList::RANGE:
      {
        IterExprWireRange* range = list->range(i);
        wtk::index_t const first =
          wtk::utils::iterExprEval(range->first(), iters_map);
        wtk::index_t const last =
          wtk::utils::iterExprEval(range->last(), iters_map);

        for(wtk::index_t j = first; j <= last; j++)
        {
          std::string edge("s");
          edge += wtk::utils::dec(loop_scope);
          edge += "mi";
          edge += wtk::utils::dec(i);
          edge += " -> ";
          edge += "s";
          edge += wtk::utils::dec(cur_scope);
          edge += "w";
          edge += wtk::utils::dec(wire_num);
          edges.insert(edge);
          wire_num++;
        }
        break;
      }
      }
    }
  }
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printLoopInputEdges(
    IterExprWireList* list, size_t const output_count, size_t const loop_scope)
{
  std::unordered_map<std::string, wtk::index_t> iters_map;
  for(size_t i = 0; i < this->loopBounds.size(); i++)
  {
    std::string n(this->loopBounds[i].name);
    iters_map[n] = this->loopBounds[i].first;
  }

  std::unordered_set<std::string> edges;

  printLoopInputEdgesHelper(
      list, output_count, loop_scope, this->curScope,
      iters_map, edges, this->loopBounds, 0);

  for(std::string edge : edges)
  {
    if(this->useFg)
    {
      fprintf(this->outFile, "%s%s [color=\"%s\"];\n",
          this->indent.get(), edge.c_str(), this->foreground.c_str());
    }
    else
    {
      fprintf(this->outFile, "%s%s;\n", this->indent.get(), edge.c_str());
    }
  }
}

inline void printLoopOutputEdgesHelper(
    IterExprWireList* list, size_t const loop_scope, size_t const cur_scope,
    std::unordered_map<std::string, wtk::index_t>& iters_map,
    std::unordered_set<std::string>& edges,
    std::vector<LoopBound>& bounds, size_t const bounds_idx)
{
  if(bounds_idx < bounds.size())
  {
    LoopBound* bound = &bounds[bounds_idx];
    std::string n(bound->name);
    for(wtk::index_t i = bound->first; i <= bound->last; i++)
    {
      iters_map[n] = i;
      printLoopOutputEdgesHelper(list, loop_scope, cur_scope, iters_map,
          edges, bounds, bounds_idx + 1);
    }
  }
  else
  {
    wtk::index_t wire_num = 0;
    for(size_t i = 0; i < list->size(); i++)
    {
      switch(list->type(i))
      {
      case IterExprWireList::SINGLE:
      {
        std::string edge("s");
        edge += wtk::utils::dec(cur_scope);
        edge += "w";
        edge += wtk::utils::dec(wire_num);
        edge += " -> ";
        edge += "s";
        edge += wtk::utils::dec(loop_scope);
        edge += "mo";
        edge += wtk::utils::dec(i);
        edges.insert(edge);
        wire_num++;
        break;
      }
      case IterExprWireList::RANGE:
      {
        IterExprWireRange* range = list->range(i);
        wtk::index_t const first =
          wtk::utils::iterExprEval(range->first(), iters_map);
        wtk::index_t const last =
          wtk::utils::iterExprEval(range->last(), iters_map);

        for(wtk::index_t j = first; j <= last; j++)
        {
          std::string edge("s");
          edge += wtk::utils::dec(cur_scope);
          edge += "w";
          edge += wtk::utils::dec(wire_num);
          edge += " -> ";
          edge += "s";
          edge += wtk::utils::dec(loop_scope);
          edge += "mo";
          edge += wtk::utils::dec(i);
          edges.insert(edge);
          wire_num++;
        }
        break;
      }
      }
    }
  }
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printLoopOutputEdges(
    IterExprWireList* list, size_t const loop_scope)
{
  std::unordered_map<std::string, wtk::index_t> iters_map;
  for(size_t i = 0; i < this->loopBounds.size(); i++)
  {
    std::string n(this->loopBounds[i].name);
    iters_map[n] = this->loopBounds[i].first;
  }

  std::unordered_set<std::string> edges;

  printLoopOutputEdgesHelper(
      list, loop_scope, this->curScope, iters_map, edges, this->loopBounds, 0);

  for(std::string edge : edges)
  {
    if(this->useFg)
    {
      fprintf(this->outFile, "%s%s [color=\"%s\"];\n",
          this->indent.get(), edge.c_str(), this->foreground.c_str());
    }
    else
    {
      fprintf(this->outFile, "%s%s;\n", this->indent.get(), edge.c_str());
    }
  }
}

inline void printLoopOutputMappingEdgesHelper(
    IterExprWireList* list, size_t const loop_scope, size_t const cur_scope,
    std::unordered_map<std::string, wtk::index_t>& iters_map,
    std::unordered_set<std::string>& edges,
    std::vector<LoopBound>& bounds, size_t const bounds_idx)
{
  if(bounds_idx < bounds.size())
  {
    LoopBound* bound = &bounds[bounds_idx];
    std::string n(bound->name);
    for(wtk::index_t i = bound->first; i <= bound->last; i++)
    {
      iters_map[n] = i;
      printLoopOutputMappingEdgesHelper(list, loop_scope, cur_scope, iters_map,
          edges, bounds, bounds_idx + 1);
    }
  }
  else
  {
    for(size_t i = 0; i < list->size(); i++)
    {
      switch(list->type(i))
      {
      case IterExprWireList::SINGLE:
      {
        wtk::index_t idx = wtk::utils::iterExprEval(list->single(i), iters_map);
        std::string edge("s");
        edge += wtk::utils::dec(loop_scope);
        edge += "mo";
        edge += wtk::utils::dec(i);
        edge += " -> ";
        edge += "s";
        edge += wtk::utils::dec(cur_scope);
        edge += "w";
        edge += wtk::utils::dec(idx);
        edges.insert(edge);
        break;
      }
      case IterExprWireList::RANGE:
      {
        IterExprWireRange* range = list->range(i);
        wtk::index_t const first =
          wtk::utils::iterExprEval(range->first(), iters_map);
        wtk::index_t const last =
          wtk::utils::iterExprEval(range->last(), iters_map);

        for(wtk::index_t j = first; j <= last; j++)
        {
          std::string edge("s");
          edge += wtk::utils::dec(loop_scope);
          edge += "mo";
          edge += wtk::utils::dec(i);
          edge += " -> ";
          edge += "s";
          edge += wtk::utils::dec(cur_scope);
          edge += "w";
          edge += wtk::utils::dec(j);
          edges.insert(edge);
        }
        break;
      }
      }
    }
  }
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printLoopOutputMappingEdges(
    IterExprWireList* list, size_t const loop_scope)
{
  std::unordered_map<std::string, wtk::index_t> iters_map;
  for(size_t i = 0; i < this->loopBounds.size(); i++)
  {
    std::string n(this->loopBounds[i].name);
    iters_map[n] = this->loopBounds[i].first;
  }

  std::unordered_set<std::string> edges;

  printLoopOutputMappingEdgesHelper(
      list, loop_scope, this->curScope, iters_map, edges, this->loopBounds, 0);

  for(std::string edge : edges)
  {
    if(this->useFg)
    {
      fprintf(this->outFile, "%s%s [color=\"%s\"];\n",
          this->indent.get(), edge.c_str(), this->foreground.c_str());
    }
    else
    {
      fprintf(this->outFile, "%s%s;\n", this->indent.get(), edge.c_str());
    }
  }
}

inline void printLoopInputMappingEdgesHelper(
    IterExprWireList* list, size_t const loop_scope, size_t const cur_scope,
    std::unordered_map<std::string, wtk::index_t>& iters_map,
    std::unordered_set<std::string>& edges,
    std::vector<LoopBound>& bounds, size_t const bounds_idx)
{
  if(bounds_idx < bounds.size())
  {
    LoopBound* bound = &bounds[bounds_idx];
    std::string n(bound->name);
    for(wtk::index_t i = bound->first; i <= bound->last; i++)
    {
      iters_map[n] = i;
      printLoopInputMappingEdgesHelper(list, loop_scope, cur_scope, iters_map,
          edges, bounds, bounds_idx + 1);
    }
  }
  else
  {
    for(size_t i = 0; i < list->size(); i++)
    {
      switch(list->type(i))
      {
      case IterExprWireList::SINGLE:
      {
        wtk::index_t idx = wtk::utils::iterExprEval(list->single(i), iters_map);
        std::string edge("s");
        edge += wtk::utils::dec(cur_scope);
        edge += "w";
        edge += wtk::utils::dec(idx);
        edge += " -> ";
        edge += "s";
        edge += wtk::utils::dec(loop_scope);
        edge += "mi";
        edge += wtk::utils::dec(i);
        edges.insert(edge);
        break;
      }
      case IterExprWireList::RANGE:
      {
        IterExprWireRange* range = list->range(i);
        wtk::index_t const first =
          wtk::utils::iterExprEval(range->first(), iters_map);
        wtk::index_t const last =
          wtk::utils::iterExprEval(range->last(), iters_map);

        for(wtk::index_t j = first; j <= last; j++)
        {
          std::string edge("s");
          edge += wtk::utils::dec(cur_scope);
          edge += "w";
          edge += wtk::utils::dec(j);
          edge += " -> ";
          edge += "s";
          edge += wtk::utils::dec(loop_scope);
          edge += "mi";
          edge += wtk::utils::dec(i);
          edges.insert(edge);
        }
        break;
      }
      }
    }
  }
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printLoopInputMappingEdges(
    IterExprWireList* list, size_t const loop_scope)
{
  std::unordered_map<std::string, wtk::index_t> iters_map;
  for(size_t i = 0; i < this->loopBounds.size(); i++)
  {
    std::string n(this->loopBounds[i].name);
    iters_map[n] = this->loopBounds[i].first;
  }

  std::unordered_set<std::string> edges;

  printLoopInputMappingEdgesHelper(
      list, loop_scope, this->curScope, iters_map, edges, this->loopBounds, 0);

  for(std::string edge : edges)
  {
    if(this->useFg)
    {
      fprintf(this->outFile, "%s%s [color=\"%s\"];\n",
          this->indent.get(), edge.c_str(), this->foreground.c_str());
    }
    else
    {
      fprintf(this->outFile, "%s%s;\n", this->indent.get(), edge.c_str());
    }
  }
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printForLoop(ForLoop<Number_T>* loop)
{
  size_t const outer_scope = this->curScope;
  size_t const loop_scope = ++this->scopeNum;
  this->curScope = ++this->scopeNum;
  fprintf(this->outFile, "%ssubgraph cluster_s%zu {\n  %slabel=\"@for %s "
      "@first %" PRIu64 " @last %" PRIu64 "\";\n", this->indent.get(),
      loop_scope, this->indent.get(), loop->iterName(), loop->first(),
      loop->last());
  this->indent.inc();

  if(this->useFg)
  {
    fprintf(this->outFile, "%scolor=\"%s\";\n%sfontcolor=\"%s\";\n",
        this->indent.get(), this->foreground.c_str(),
        this->indent.get(), this->foreground.c_str());
  }

  this->loopBounds.emplace_back(loop->iterName(), loop->first(), loop->last());

  IterExprWireList* inputs = nullptr;
  IterExprWireList* outputs = nullptr;

  switch(loop->bodyType())
  {
  case ForLoop<Number_T>::INVOKE:
  {
    IterExprFunctionInvoke* invoke = loop->invokeBody();
    inputs = invoke->inputList();
    outputs = invoke->outputList();

    size_t output_count =
      this->printLoopOutputMappingNodes(outputs, loop_scope);
    this->printLoopInputMappingNodes(inputs, loop_scope);

    fprintf(this->outFile,
        "%ssubgraph cluster_s%zu {\n  %slabel=\"@call(%s)\";\n",
        this->indent.get(), this->curScope, this->indent.get(),
        invoke->name());
    this->indent.inc();

    if(this->useFg)
    {
      fprintf(this->outFile, "%scolor=\"%s\";\n%sfontcolor=\"%s\";\n",
          this->indent.get(), this->foreground.c_str(),
          this->indent.get(), this->foreground.c_str());
    }

    size_t input_count = this->printLoopInputNodes(inputs, output_count);

    fprintf(this->outFile, "%scall%zu [label=\"call(%s)\"];\n",
        this->indent.get(), this->curScope, invoke->name());

    for(wtk::index_t i = (wtk::index_t) output_count; i <= (wtk::index_t) input_count; i++)
    {
      fprintf(this->outFile, "%ss%zuw%" PRIu64 " -> call%zu;\n",
          this->indent.get(), this->curScope, i, this->curScope);
    }

    for(wtk::index_t i = 0; i < (wtk::index_t) output_count; i++)
    {
      if(this->useFg)
      {
        fprintf(this->outFile,
            "%ss%zuw%" PRIu64 " [label=\"out -> $%" PRIu64 "\", "
            "color=\"%s\", fontcolor=\"%s\"];\n"
            "%scall%zu -> s%zuw%" PRIu64 " [color=\"%s\";\n",
            this->indent.get(), this->curScope, i, i, this->foreground.c_str(),
            this->foreground.c_str(), this->indent.get(), this->curScope,
            this->curScope, i, this->foreground.c_str());
      }
      else
      {
        fprintf(this->outFile,
            "%ss%zuw%" PRIu64 " [label=\"out -> $%" PRIu64 "\"];\n"
            "%scall%zu -> s%zuw%" PRIu64 ";\n",
            this->indent.get(), this->curScope, i, i,
            this->indent.get(), this->curScope, this->curScope, i);
      }
    }

    this->indent.dec();
    fprintf(this->outFile, "%s}\n", this->indent.get());

    this->printLoopInputEdges(inputs, output_count, loop_scope);
    this->printLoopOutputEdges(outputs, loop_scope);

    break;
  }
  case ForLoop<Number_T>::ANONYMOUS:
  {
    IterExprAnonFunction<Number_T>* anon = loop->anonymousBody();
    inputs = anon->inputList();
    outputs = anon->outputList();

    size_t const output_count =
      this->printLoopOutputMappingNodes(outputs, loop_scope);
    this->printLoopInputMappingNodes(inputs, loop_scope);
    fprintf(this->outFile,
        "%ssubgraph cluster_s%zu {\n  %slabel=\"@anon_call\";\n",
        this->indent.get(), this->curScope, this->indent.get());
    this->indent.inc();

    if(this->useFg)
    {
      fprintf(this->outFile, "%scolor=\"%s\";\n%sfontcolor=\"%s\";\n",
          this->indent.get(), this->foreground.c_str(),
          this->indent.get(), this->foreground.c_str());
    }

    this->printLoopInputNodes(inputs, output_count);
    this->indent.dec();

    this->printDirectiveList(anon->body());

    fprintf(this->outFile, "%s}\n", this->indent.get());

    this->printLoopInputEdges(inputs, output_count, loop_scope);
    this->printLoopOutputEdges(outputs, loop_scope);

    break;
  }
  }

  this->indent.dec();
  fprintf(this->outFile, "%s}\n", this->indent.get());

  for(size_t i = 0; i < loop->outputList()->size(); i++)
  {
    switch(loop->outputList()->type(i))
    {
    case WireList::SINGLE:
    {
      index_t idx = loop->outputList()->single(i);
      if(this->useFg)
      {
        fprintf(this->outFile,
            "%ss%zuw%" PRIu64 "[label=\"out: $%" PRIu64 "\", "
            "color=\"%s\", fontcolor=\"%s\"];\n",
            this->indent.get(), outer_scope, idx, idx,
            this->foreground.c_str(), this->foreground.c_str());
      }
      else
      {
        fprintf(this->outFile,
            "%ss%zuw%" PRIu64 "[label=\"out: $%" PRIu64 "\"];\n",
            this->indent.get(), outer_scope, idx, idx);
      }
      break;
    }
    case WireList::RANGE:
    {
      wtk::index_t first = loop->outputList()->range(i)->first();
      wtk::index_t last = loop->outputList()->range(i)->last();

      for(wtk::index_t j = first; j <= last; j++)
      {
        fprintf(this->outFile,
            "%ss%zuw%" PRIu64 "[label=\"out: $%" PRIu64 "\", "
            "color=\"%s\", fontcolor=\"%s\"];\n",
            this->indent.get(), outer_scope, j, j, this->foreground.c_str(),
            this->foreground.c_str());
      }
      break;
    }
    }
  }

  this->curScope = outer_scope;
  this->printLoopOutputMappingEdges(outputs, loop_scope);
  this->printLoopInputMappingEdges(inputs, loop_scope);
  this->loopBounds.pop_back();
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printSwitchStatement(
    SwitchStatement<Number_T>* switch_stmt)
{
  size_t const outer_scope = this->curScope;
  size_t const switch_scope = ++this->scopeNum;

  size_t const output_count = this->countWireList(switch_stmt->outputList());

  for(size_t i = 0; i < switch_stmt->size(); i++)
  {
    this->curScope = switch_scope + 1 + i;
    CaseBlock<Number_T>* case_blk = switch_stmt->caseBlock(i);

    switch(case_blk->bodyType())
    {
    case CaseBlock<Number_T>::INVOKE:
    {
      this->printInputListEdges(
          case_blk->invokeBody()->inputList(), outer_scope, output_count);
      break;
    }
    case CaseBlock<Number_T>::ANONYMOUS:
    {
      this->printInputListEdges(
          case_blk->anonymousBody()->inputList(), outer_scope, output_count);
      break;
    }
    }
  }
  this->curScope = outer_scope;

  if(this->useFg)
  {
    fprintf(this->outFile, "%ss%zuw%" PRIu64 " -> s%zucond [color=\"%s\"];\n",
        this->indent.get(), outer_scope, switch_stmt->condition(),
        switch_scope, this->foreground.c_str());
  }
  else
  {
    fprintf(this->outFile, "%ss%zuw%" PRIu64 " -> s%zucond;\n",
        this->indent.get(), outer_scope, switch_stmt->condition(),
        switch_scope);
  }

  fprintf(this->outFile,
      "%ssubgraph cluster_s%zu {\n  %slabel=\"@switch($%" PRIu64 ")\";\n",
      this->indent.get(), switch_scope,
      this->indent.get(), switch_stmt->condition());
  this->indent.inc();

  if(this->useFg)
  {
    fprintf(this->outFile, "%scolor=\"%s\";\n%sfontcolor=\"%s\";\n",
        this->indent.get(), this->foreground.c_str(),
        this->indent.get(), this->foreground.c_str());
  }

  if(this->useFg)
  {
    fprintf(this->outFile, "%ss%zucond [label=\"CONDITION\", "
        "color=\"%s\", fontcolor=\"%s\"];\n",
        this->indent.get(), switch_scope, this->foreground.c_str(),
        this->foreground.c_str());
  }
  else
  {
    fprintf(this->outFile, "%ss%zucond [label=\"CONDITION\"];\n",
        this->indent.get(), switch_scope);
  }

  for(size_t i = 0; i < switch_stmt->size(); i++)
  {
    this->curScope = ++this->scopeNum;
    CaseBlock<Number_T>* case_blk = switch_stmt->caseBlock(i);

    fprintf(this->outFile,
        "%ssubgraph cluster_s%zu {\n  %slabel=\"@case < %s >:\";\n",
        this->indent.get(), this->curScope,
        this->indent.get(), wtk::utils::dec(case_blk->match()).c_str());
    this->indent.inc();

    if(this->useFg)
    {
      fprintf(this->outFile, "%scolor=\"%s\";\n%sfontcolor=\"%s\";\n",
          this->indent.get(), this->foreground.c_str(),
          this->indent.get(), this->foreground.c_str());
    }

    this->curScope = switch_scope + 1 + i;

    switch(case_blk->bodyType())
    {
    case CaseBlock<Number_T>::INVOKE:
    {
      CaseFunctionInvoke* invoke = case_blk->invokeBody();

      this->printInputListNodes(invoke->inputList(), output_count);
      if(this->useFg)
      {
        fprintf(this->outFile, "%scall%zu [label=\"@call(%s)\", "
            "color=\"%s\", fontcolor=\"%s\"];\n",
            this->indent.get(), this->curScope, invoke->name(),
            this->foreground.c_str(), this->foreground.c_str());
      }
      else
      {
        fprintf(this->outFile, "%scall%zu [label=\"@call(%s)\"];\n",
            this->indent.get(), this->curScope, invoke->name());
      }

      size_t const input_count = this->countWireList(invoke->inputList());
      for(wtk::index_t i = (wtk::index_t) output_count; i <= (wtk::index_t) input_count; i++)
      {
        if(this->useFg)
        {
          fprintf(this->outFile,
              "%ss%zuw%" PRIu64 " -> call%zu [color=\"%s\"];\n",
              this->indent.get(), this->curScope, i, this->curScope,
              this->foreground.c_str());
        }
        else
        {
          fprintf(this->outFile, "%ss%zuw%" PRIu64 " -> call%zu;\n",
              this->indent.get(), this->curScope, i, this->curScope);
        }
      }

      for(wtk::index_t i = 0; i < (wtk::index_t) output_count; i++)
      {
        if(this->useFg)
        {
          fprintf(this->outFile,
              "%ss%zuw%" PRIu64 " [label=\"out -> $%" PRIu64 "\","
              "color=\"%s\", fontcolor=\"%s\"];\n"
              "%scall%zu -> s%zuw%" PRIu64 " [color=\"%s\"];\n",
              this->indent.get(), this->curScope, i, i,
              this->foreground.c_str(), this->foreground.c_str(),
              this->indent.get(), this->curScope, this->curScope, i,
              this->foreground.c_str());
        }
        else
        {
          fprintf(this->outFile,
              "%ss%zuw%" PRIu64 " [label=\"out -> $%" PRIu64 "\"];\n"
              "%scall%zu -> s%zuw%" PRIu64 ";\n",
              this->indent.get(), this->curScope, i, i,
              this->indent.get(), this->curScope, this->curScope, i);
        }

      }

      break;
    }
    case CaseBlock<Number_T>::ANONYMOUS:
    {
      CaseAnonFunction<Number_T>* anon = case_blk->anonymousBody();
      this->printInputListNodes(anon->inputList(), output_count);
      this->printDirectiveList(anon->body());
      break;
    }
    }

    this->indent.dec();
    fprintf(this->outFile, "%s}\n", this->indent.get());
  }

  this->indent.dec();
  fprintf(this->outFile, "%s}\n", this->indent.get());

  WireList* outputs = switch_stmt->outputList();
  for(size_t i = 0; i < outputs->size(); i++)
  {
    switch(outputs->type(i))
    {
    case WireList::SINGLE:
    {
      if(this->useFg)
      {
        fprintf(this->outFile, "%ss%zuw%" PRIu64 " [label=\"out: $%" PRIu64
            "\", color=\"%s\", fontcolor=\"%s\"];\n", this->indent.get(),
            outer_scope, outputs->single(i), outputs->single(i),
            this->foreground.c_str(), this->foreground.c_str());
      }
      else
      {
        fprintf(this->outFile,
            "%ss%zuw%" PRIu64 " [label=\"out: $%" PRIu64 "\"];\n",
            this->indent.get(), outer_scope, outputs->single(i),
            outputs->single(i));
      }
      break;
    }
    case WireList::RANGE:
    {
      WireRange* range = outputs->range(i);
      wtk::index_t first = range->first();
      wtk::index_t last = range->last();
      for(wtk::index_t j = first; j <= last; j++)
      {
        if(this->useFg)
        {
          fprintf(this->outFile, "%ss%zuw%" PRIu64 " [label=\"out: $%" PRIu64
              "\", color=\"%s\", fontcolor=\"%s\"];\n", this->indent.get(),
              outer_scope, j, j, this->foreground.c_str(),
              this->foreground.c_str());
        }
        else
        {
          fprintf(this->outFile,
              "%ss%zuw%" PRIu64 " [label=\"out: $%" PRIu64 "\"];\n",
              this->indent.get(), outer_scope, j, j);
        }
      }
      break;
    }
    }
  }

  for(size_t i = 0; i < switch_stmt->size(); i++)
  {
    this->curScope = switch_scope + 1 + i;

    wtk::index_t inner_wire = 0;
    for(size_t j = 0; j < outputs->size(); j++)
    {
      switch(outputs->type(j))
      {
      case WireList::SINGLE:
      {
        if(this->useFg)
        {
          fprintf(this->outFile, "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64
              "[color=\"%s\"];\n", this->indent.get(), this->curScope,
              inner_wire, outer_scope, outputs->single(j),
              this->foreground.c_str());
        }
        else
        {
          fprintf(this->outFile, "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 ";\n",
              this->indent.get(), this->curScope, inner_wire, outer_scope,
              outputs->single(j));
        }
        inner_wire++;
        break;
      }
      case WireList::RANGE:
      {
        WireRange* range = outputs->range(i);
        wtk::index_t first = range->first();
        wtk::index_t last = range->last();
        for(wtk::index_t k = first; k <= last; k++)
        {
          if(this->useFg)
          {
            fprintf(this->outFile, "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64
                " [color=\"%s\"];\n",
                this->indent.get(), this->curScope, inner_wire, outer_scope,
                k, this->foreground.c_str());
          }
          else
          {
            fprintf(this->outFile, "%ss%zuw%" PRIu64 " -> s%zuw%" PRIu64 ";\n",
                this->indent.get(), this->curScope, inner_wire, outer_scope,
                k);
          }
          inner_wire++;
        }
        break;
      }
      }
    }
  }
  this->curScope = outer_scope;
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printDirectiveList(
    DirectiveList<Number_T>* list)
{
  this->indent.inc();

  for(size_t i = 0; i < list->size(); i++)
  {
    switch(list->type(i))
    {
    case DirectiveList<Number_T>::BINARY_GATE:
    {
      this->printBinaryGate(list->binaryGate(i));
      break;
    }
    case DirectiveList<Number_T>::BINARY_CONST_GATE:
    {
      this->printBinaryConstGate(list->binaryConstGate(i));
      break;
    }
    case DirectiveList<Number_T>::UNARY_GATE:
    {
      this->printUnaryGate(list->unaryGate(i));
      break;
    }
    case DirectiveList<Number_T>::INPUT:
    {
      this->printInput(list->input(i));
      break;
    }
    case DirectiveList<Number_T>::ASSIGN:
    {
      this->printAssign(list->assign(i));
      break;
    }
    case DirectiveList<Number_T>::ASSERT_ZERO:
    {
      this->printAssertZero(list->assertZero(i));
      break;
    }
    case DirectiveList<Number_T>::DELETE_SINGLE:
    {
      this->printDeleteSingle(list->deleteSingle(i));
      break;
    }
    case DirectiveList<Number_T>::DELETE_RANGE:
    {
      this->printDeleteRange(list->deleteRange(i));
      break;
    }
    case DirectiveList<Number_T>::FUNCTION_INVOKE:
    {
      this->printFunctionInvoke(list->functionInvoke(i));
      break;
    }
    case DirectiveList<Number_T>::ANON_FUNCTION:
    {
      this->printAnonFunction(list->anonFunction(i));
      break;
    }
    case DirectiveList<Number_T>::FOR_LOOP:
    {
      this->printForLoop(list->forLoop(i));
      break;
    }
    case DirectiveList<Number_T>::SWITCH_STATEMENT:
    {
      this->printSwitchStatement(list->switchStatement(i));
      break;
    }
    }
  }

  this->indent.dec();
}

template<typename Number_T>
void TreeVisualizer<Number_T>::printTree(IRTree<Number_T>* tree)
{
  fprintf(this->outFile, "digraph Rel \n{\n  rankdir=LR;\n");
  this->indent.inc();

  if(this->useBg)
  {
    fprintf(this->outFile, "%sbgcolor=\"%s\";\n",
        this->indent.get(), this->background.c_str());
  }

  for(size_t i = 0; i < tree->size(); i++)
  {
    FunctionDeclare<Number_T>* func = tree->functionDeclare(i);
    this->curScope = ++this->scopeNum;

    fprintf(this->outFile,
        "%ssubgraph cluster_function%zu {\n  %slabel=\"@function(%s)\";\n",
        this->indent.get(), this->curScope, this->indent.get(), func->name());

    if(this->useFg)
    {
      fprintf(this->outFile, "  %scolor=\"%s\";\n  %sfontcolor=\"%s\";\n",
          this->indent.get(), this->foreground.c_str(),
          this->indent.get(), this->foreground.c_str());
    }

    for(wtk::index_t j = func->outputCount(); j <= func->inputCount(); j++)
    {
      if(this->useFg)
      {
        fprintf(this->outFile,
            "  %ss%zuw%" PRIu64 "[label=\"in: $%" PRIu64 "\", "
            "color=\"%s\", fontcolor=\"%s\"];\n",
            this->indent.get(), this->curScope, j, j,
            this->foreground.c_str(), this->foreground.c_str());
      }
      else
      {
        fprintf(this->outFile,
            "  %ss%zuw%" PRIu64 "[label=\"in: $%" PRIu64 "\"];\n",
            this->indent.get(), this->curScope, j, j);
      }
    }

    this->printDirectiveList(func->body());

    fprintf(this->outFile, "%s}\n", this->indent.get());
  }
  this->indent.dec();
  this->curScope = 0;

  this->printDirectiveList(tree->body());
  fprintf(this->outFile, "}\n");
}

} } // namespace wtk::viz
