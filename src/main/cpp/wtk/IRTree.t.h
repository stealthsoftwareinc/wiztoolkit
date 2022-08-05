/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {

template<typename Number_T>
size_t BinaryConstGate<Number_T>::lineNum() { return 0; }

template<typename Number_T>
size_t Assign<Number_T>::lineNum() { return 0; }

template<typename Number_T>
size_t FunctionDeclare<Number_T>::lineNum() { return 0; }

template<typename Number_T>
size_t AnonFunction<Number_T>::lineNum() { return 0; }

template<typename Number_T>
size_t IterExprAnonFunction<Number_T>::lineNum() { return 0; }

template<typename Number_T>
size_t ForLoop<Number_T>::lineNum() { return 0; }

template<typename Number_T>
size_t CaseAnonFunction<Number_T>::lineNum() { return 0; }

template<typename Number_T>
size_t CaseBlock<Number_T>::lineNum() { return 0; }

template<typename Number_T>
size_t SwitchStatement<Number_T>::lineNum() { return 0; }

template<typename Number_T>
size_t DirectiveList<Number_T>::lineNum() { return 0; }

template<typename Number_T>
size_t IRTree<Number_T>::lineNum() { return 0; }

} // namespace wtk
