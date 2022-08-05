/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_ABSTRACT_IR_TREE_H_
#define WTK_ABSTRACT_IR_TREE_H_

#include <cstdint>
#include <cstddef>

#include <wtk/index.h>

namespace wtk {

/**
 * These are a bunch of abstract classes that make up a common interface
 * to the syntax tree of an IR Relation. 
 *
 * Various parsers can implement these abstract classes with various
 * backing structures.
 */

/**
 * This represents a binary gate in the IR. Binary refers to the two-input
 * wires of this gate, not numeric representation.
 */
struct BinaryGate
{
  /**
   * Enumeration of allowable binary gate calculations.
   */
  enum Calculation
  {
    AND,
    XOR,
    ADD,
    MUL
  };

  /**
   * Returns the calculation which this binary gate makes.
   */
  virtual Calculation calculation() = 0;

  /**
   * Returns the output wire of the gate.
   */
  virtual index_t outputWire() = 0;

  /**
   * Returns the left input wire of the gate.
   */
  virtual index_t leftWire() = 0;

  /**
   * Returns the right input wire of the gate.
   */
  virtual index_t rightWire() = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~BinaryGate() = default;
};

/**
 * This represents a unary gate in the IR. Unary refers to the two-input
 * wires of this gate, not numeric representation.
 */
struct UnaryGate
{
  /**
   * Enumeration of allowable unary gate calculations.
   */
  enum Calculation
  {
    NOT,
    COPY
  };

  /**
   * Returns the calculation which this unary gate makes.
   */
  virtual Calculation calculation() = 0;

  /**
   * Returns the output wire of the gate.
   */
  virtual index_t outputWire() = 0;

  /**
   * Returns the input wire of the gate.
   */
  virtual index_t inputWire() = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~UnaryGate() = default;
};


/**
 * This represents a binary constant gate in the IR. Binary refers to the
 * two-inputs of this gate, not numeric representation. The second input of
 * these gates is a constant value instead of a wire.
 */
template<typename Number_T>
struct BinaryConstGate
{
  /**
   * Enumeration of allowable binary const gate calculations.
   */
  enum Calculation
  {
    ADDC,
    MULC
  };

  /**
   * Returns the calculation which this binary const gate makes.
   */
  virtual Calculation calculation() = 0;

  /**
   * Returns the output wire of the gate.
   */
  virtual index_t outputWire() = 0;

  /**
   * Returns the left input wire of the gate.
   */
  virtual index_t leftWire() = 0;

  /**
   * Returns the right constant-input of the gate.
   */
  virtual Number_T rightValue() = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~BinaryConstGate() = default;
};

/**
 * Reperesents an input directive.
 */
struct Input
{
  enum Stream
  {
    INSTANCE,
    SHORT_WITNESS
  };

  /**
   * Indicates which stream this directive reads for input.
   */
  virtual Stream stream() = 0;

  /**
   * Returns the output wire of the input directive.
   */
  virtual index_t outputWire() = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~Input() = default;
};

/**
 * Represents a constant assignment directive.
 */
template<typename Number_T>
struct Assign
{
  /**
   * Returns the output wire of this assign directive.
   */
  virtual index_t outputWire() = 0;

  /**
   * Returns the constant value set by this directive.
   */
  virtual Number_T constValue() = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~Assign() = default;
};

/**
 * Represents a directive with just a single input wire.
 */
struct Terminal
{
  /**
   * Returns the single input wire of this terminal.
   */
  virtual index_t wire() = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~Terminal() = default;
};

/**
 * A range of wires, from first to last inclusive.
 */
struct WireRange
{
  /**
   * the first wire in the range.
   */
  virtual index_t first() = 0;

  /**
   * The last wire in the range.
   */
  virtual index_t last() = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~WireRange() = default;
};

/**
 * This is a list of wires. Its elements are either single wires or
 * ranges of wires.
 */
struct WireList
{
  enum Type
  {
    SINGLE,
    RANGE
  };

  /**
   * Returns the number of list elements (not wire total) in this wire list.
   */
  virtual size_t size() = 0;

  /**
   * Returns the type of the nth element.
   */
  virtual Type type(size_t n) = 0;

  /**
   * Returns the nth element as a single index. If this method
   * is invoked on an n where n >= size() or type(n) != SINGLE, the
   * behavior is undefined.
   */
  virtual index_t single(size_t n) = 0;

  /**
   * Returns the nth element as a range. If this method is invoked on
   * an n where n >= size() or type(n) != RANGE, the behavior is
   * undefined.
   */
  virtual WireRange* range(size_t n) = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~WireList() = default;
};

template<typename Number_T>
struct DirectiveList;

/**
 * This represents a function-gate declaration.
 */
template<typename Number_T>
struct FunctionDeclare
{
  /**
   * Returns the name of the function gate declaration.
   */
  virtual char const* name() = 0;

  /**
   * Returns the number of outputs provided by the function.
   */
  virtual index_t outputCount() = 0;

  /**
   * Returns the number of inputs required by the function.
   */
  virtual index_t inputCount() = 0;

  /**
   * Returns the number of instance values consumed by the function.
   */
  virtual index_t instanceCount() = 0;

  /**
   * Returns the number of short witness values consumed by the function.
   */
  virtual index_t shortWitnessCount() = 0;

  /**
   * Returns the list of directives defining the function body.
   */
  virtual DirectiveList<Number_T>* body() = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~FunctionDeclare() = default;
};

/**
 * This represents a function gate invocation.
 */
struct FunctionInvoke
{
  /**
   * Returns the name of the funciton being invoked.
   */
  virtual char const* name() = 0;

  /**
   * Returns the output list of the function invocation.
   * Should be nonnull even if it is empty.
   */
  virtual WireList* outputList() = 0;

  /**
   * Returns the input list of the function invocation.
   * Should be nonnull even if it is empty.
   */
  virtual WireList* inputList() = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~FunctionInvoke() = default;
};

/**
 * This represents an anonymous function invocation.
 */
template<typename Number_T>
struct AnonFunction
{
  /**
   * Returns the output list of the anonymous function.
   * Should be nonnull even if it is empty.
   */
  virtual WireList* outputList() = 0;

  /**
   * Returns the input list of the anonymous function.
   * Should be nonnull even if it is empty.
   */
  virtual WireList* inputList() = 0;

  /**
   * Returns the number of instance values consumed by the anonymous function.
   */
  virtual index_t instanceCount() = 0;

  /**
   * Returns the number of short witness values consumed by the anonymous
   * function.
   */
  virtual index_t shortWitnessCount() = 0;

  /**
   * Returns the list of directives defining the anonymous function body.
   */
  virtual DirectiveList<Number_T>* body() = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~AnonFunction() = default;
};

/**
 * This represents an for-loop iterator expression.
 */
struct IterExpr
{
  enum Type
  {
    LITERAL,
    ITERATOR,
    ADD,
    SUB,
    MUL,
    DIV
  };

  /**
   * Returns if the expression is one of a literal, a name, or an
   * integer expression.
   */
  virtual Type type() = 0;

  /**
   * If the expression is a literal, this returns that value. If it
   * is a div, then it returns the literal right-hand-side, otherwise
   * its behavior is undefined.
   */
  virtual index_t literal() = 0;

  /**
   * If the expression is a name it returns a char* pointing
   * to the name referenced. Otherwise its behavior is undefined.
   */
  virtual char const* name() = 0;

  /**
   * If the expression is one of add, sub, mul, or div, then this returns
   * the left-hand-side of the expression. Otherwise the behavior is
   * undefined.
   */
  virtual IterExpr* lhs() = 0;

  /**
   * If the expression is one of add, sub, or mul, then this returns
   * the right-hand-side of the expression. Otherwise the behavior is
   * undefined.
   */
  virtual IterExpr* rhs() = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~IterExpr() = default;
};

/**
 * A wire range which uses iterator expressions instead of exact
 * indexes. This is used for mapping wires into for-loop body functions.
 */
struct IterExprWireRange
{
  /**
   * the first wire in the range.
   */
  virtual IterExpr* first() = 0;

  /**
   * The last wire in the range.
   */
  virtual IterExpr* last() = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~IterExprWireRange() = default;
};

/**
 * This is a list of wires which uses iterator expressions instead of
 * exact wire numbers. Its elements are either single wires or
 * ranges of wires. This is used for mapping wires into for-loop body
 * functions.
 */
struct IterExprWireList
{
  enum Type
  {
    SINGLE,
    RANGE
  };

  /**
   * Returns the number of list elements (not wire total) in this wire list.
   */
  virtual size_t size() = 0;

  /**
   * Returns the type of the nth element.
   */
  virtual Type type(size_t n) = 0;

  /**
   * Returns the nth element as a single index. If this method
   * is invoked on an n where n >= size() or type(n) != SINGLE, the
   * behavior is undefined.
   */
  virtual IterExpr* single(size_t n) = 0;

  /**
   * Returns the nth element as a range. If this method is invoked on
   * an n where n >= size() or type(n) != RANGE, the behavior is
   * undefined.
   */
  virtual IterExprWireRange* range(size_t n) = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~IterExprWireList() = default;
};

/**
 * This represents a function gate invocation used as the body of a for loop.
 * It allows iterator expressions to be used in its inputs and outputs.
 */
struct IterExprFunctionInvoke
{
  /**
   * Returns the name of the funciton being invoked.
   */
  virtual char const* name() = 0;

  /**
   * Returns the output list of the function invocation.
   * Should be nonnull even if it is empty.
   */
  virtual IterExprWireList* outputList() = 0;

  /**
   * Returns the input list of the function invocation.
   * Should be nonnull even if it is empty.
   */
  virtual IterExprWireList* inputList() = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~IterExprFunctionInvoke() = default;
};

/**
 * This represents an anonymous function invocation used as the body of a for
 * loop. It allows iterator expressions to be used in its inputs and outputs.
 */
template<typename Number_T>
struct IterExprAnonFunction
{
  /**
   * Returns the output list of the anonymous function.
   * Should be nonnull even if it is empty.
   */
  virtual IterExprWireList* outputList() = 0;

  /**
   * Returns the input list of the anonymous function.
   * Should be nonnull even if it is empty.
   */
  virtual IterExprWireList* inputList() = 0;

  /**
   * Returns the number of instance values consumed by the anonymous function.
   */
  virtual index_t instanceCount() = 0;

  /**
   * Returns the number of short witness values consumed by the anonymous
   * function.
   */
  virtual index_t shortWitnessCount() = 0;

  /**
   * Returns the list of directives defining the anonymous function body.
   */
  virtual DirectiveList<Number_T>* body() = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~IterExprAnonFunction() = default;
};

/**
 * This represents a for-loop.
 */
template<typename Number_T>
struct ForLoop
{
  /**
   * The list of outputs.
   */
  virtual WireList* outputList() = 0;

  /**
   * The name of the iterator.
   */
  virtual char const* iterName() = 0;

  /**
   * The first iteration value of the loop.
   */
  virtual index_t first() = 0;

  /**
   * The last iteration value of the loop.
   */
  virtual index_t last() = 0;

  enum BodyType
  {
    INVOKE,
    ANONYMOUS
  };

  /**
   * Indicates whether the for-loop body is a function invoke or its an
   * anonymous function invocation.
   */
  virtual BodyType bodyType() = 0;

  /**
   * Returns the body of this for loop as an invocation of a previously
   * defined function.
   */
  virtual IterExprFunctionInvoke* invokeBody() = 0;

  /**
   * Returns the body of this for loop as an anonymous function.
   */
  virtual IterExprAnonFunction<Number_T>* anonymousBody() = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~ForLoop() = default;
};

/**
 * Case blocks have special function-invokes without output lists. The output
 * list is instead copied from the switch-statement.
 */
struct CaseFunctionInvoke
{
  /**
   * Returns the name of the funciton being invoked.
   */
  virtual char const* name() = 0;

  /**
   * Returns the input list of the function invocation.
   * Should be nonnull even if it is empty.
   */
  virtual WireList* inputList() = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~CaseFunctionInvoke() = default;
};

/**
 * Case blocks have special anonymous-functions without output lists. The
 * output list is instead copied from the switch-statement.
 */
template<typename Number_T>
struct CaseAnonFunction
{
  /**
   * Returns the input list of the anonymous function.
   * Should be nonnull even if it is empty.
   */
  virtual WireList* inputList() = 0;

  /**
   * Returns the number of instance values consumed by the anonymous function.
   */
  virtual index_t instanceCount() = 0;

  /**
   * Returns the number of short witness values consumed by the anonymous
   * function.
   */
  virtual index_t shortWitnessCount() = 0;

  /**
   * Returns the list of directives defining the anonymous function body.
   */
  virtual DirectiveList<Number_T>* body() = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~CaseAnonFunction() = default;
};

template<typename Number_T>
struct CaseBlock
{
  /**
   * Returns the number which the case matches.
   */
  virtual Number_T match() = 0;

  enum BodyType
  {
    INVOKE,
    ANONYMOUS
  };

  /**
   * Indicates whether the case-block body is a function invoke or its an
   * anonymous function invocation.
   */
  virtual BodyType bodyType() = 0;

  /**
   * Returns the body as a function invoke. Behavior is undefined if
   * bodyType() != INVOKE.
   */
  virtual CaseFunctionInvoke* invokeBody() = 0;

  /**
   * Returns the body as an anonymous function. Behavior is undefined if
   * bodyType() != ANONYMOUS.
   */
  virtual CaseAnonFunction<Number_T>* anonymousBody() = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~CaseBlock() = default;
};

template<typename Number_T>
struct SwitchStatement
{
  /**
   * The list of outputs.
   */
  virtual WireList* outputList() = 0;

  /**
   * The index of the condition variable.
   */
  virtual index_t condition() = 0;

  /**
   * Returns the number of case blocks within the switch.
   */
  virtual size_t size() = 0;

  /**
   * Returns the nth case block, indexed by order, not by case.match().
   * If n < size(), behavior is undefined.
   */
  virtual CaseBlock<Number_T>* caseBlock(size_t n) = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~SwitchStatement() = default;
};

/**
 * Encapsulates a list of directives, or a scope block in IR1.
 */
template<typename Number_T>
struct DirectiveList
{
  enum Type
  {
    BINARY_GATE,
    BINARY_CONST_GATE,
    UNARY_GATE,
    INPUT,
    ASSIGN,
    ASSERT_ZERO,
    DELETE_SINGLE,
    DELETE_RANGE,
    FUNCTION_INVOKE,
    ANON_FUNCTION,
    FOR_LOOP,
    SWITCH_STATEMENT
  };

  /**
   * Returns the length of the list.
   */
  virtual size_t size() = 0;

  /**
   * Returns the type of the nth directive.
   * Undefined behavior if n >= size().
   */
  virtual Type type(size_t n) = 0;

  /**
   * Returns the nth directive as a binary gate.
   * Undefined behavior if n >= size() or type(n) != BINARY_GATE.
   */
  virtual BinaryGate* binaryGate(size_t n) = 0;

  /**
   * Returns the nth directive as a binary constant gate.
   * Undefined behavior if n >= size() or type(n) != BINARY_CONST_GATE.
   */
  virtual BinaryConstGate<Number_T>* binaryConstGate(size_t n) = 0;

  /**
   * Returns the nth directive as a unary gate.
   * Undefined behavior if n >= size() or type(n) != UNARY_GATE.
   */
  virtual UnaryGate* unaryGate(size_t n) = 0;

  /**
   * Returns the nth directive as an input directive.
   * Undefined behavior if n >= size() or type(n) != INPUT.
   */
  virtual Input* input(size_t n) = 0;

  /**
   * Returns the nth directive as a constant assignment directive.
   * Undefined behavior if n >= size() or type(n) != ASSIGN.
   */
  virtual Assign<Number_T>* assign(size_t n) = 0;

  /**
   * Returns the nth directive as an assert zero directive.
   * Undefined behavior if n >= size() or type(n) != ASSERT_ZERO.
   */
  virtual Terminal* assertZero(size_t n) = 0;

  /**
   * Returns the nth directive as a delete directive.
   * Undefined behavior if n >= size() or type(n) != DELETE_SINGLE.
   */
  virtual Terminal* deleteSingle(size_t n) = 0;

  /**
   * Returns the nth directive as the range of a delete directive.
   * Undefined behavior if n >= size() or type(n) != DELETE_RANGE.
   */
  virtual WireRange* deleteRange(size_t n) = 0;

  /**
   * Returns the nth directive as a function invoke.
   * Undefined behavior if n >= size() or type(n) != FUNCTION_INVOKE.
   */
  virtual FunctionInvoke* functionInvoke(size_t n) = 0;

  /**
   * Returns the nth directive as an anonymous function.
   * Undefined behavior if n >= size() or type(n) != ANONYMOUS_FUNCTION.
   */
  virtual AnonFunction<Number_T>* anonFunction(size_t n) = 0;

  /**
   * Returns the nth directive as a for loop.
   * Undefined behavior if n >= size() or type(n) != FOR_LOOP.
   */
  virtual ForLoop<Number_T>* forLoop(size_t n) = 0;

  /**
   * Returns the nth directive as a switch statement.
   * Undefined behavior if n >= size() or type(n) != SWITCH_STATEMENT.
   */
  virtual SwitchStatement<Number_T>* switchStatement(size_t n) = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~DirectiveList() = default;
};

/**
 * This is the entrypoint to the IR, it starts with a list of function
 * declarations, followed by the directives defining the circuit.
 */
template<typename Number_T>
struct IRTree
{
  /**
   * Indicates how many function declarations are a part of this relation.
   */
  virtual size_t size() = 0;

  /**
   * Returns the nth function declaration. Undefined behavior if n is
   * greater than size().
   */
  virtual FunctionDeclare<Number_T>* functionDeclare(size_t n) = 0;

  /**
   * Returns the body of directives for this relation.
   */
  virtual DirectiveList<Number_T>* body() = 0;

  /**
   * Optionally returns the line number at which this was read.
   * By default it returns 0 always.
   */
  virtual size_t lineNum();

  virtual ~IRTree() = default;
};

} // namespace wtk

#include <wtk/IRTree.t.h>

#endif // WTK_ABSTRACT_IR_TREE_H_
