/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {

template<typename Number_T>
wtk::StreamStatus QueueInputStream<Number_T>::next(Number_T* num)
{
  if(UNLIKELY(this->numbers.size() == 0))
  {
    if(this->parseError) { return wtk::StreamStatus::error; }
    else                 { return wtk::StreamStatus::end;   }
  }
  else
  {
    *num = this->numbers[0];
    this->numbers.pop_front();

    if(LIKELY(!this->firstLine)) { this->lineNums.pop_front(); }
    else                         { this->firstLine = false; }

    return wtk::StreamStatus::success;
  }
}

template<typename Number_T>
size_t QueueInputStream<Number_T>::lineNum()
{
  if(LIKELY(this->lineNums.size() != 0)) { return this->lineNums[0]; }
  else                                   { return 0; }
}

template<typename Number_T>
void QueueInputStream<Number_T>::insert(Number_T num, size_t line)
{
  this->numbers.push_back(num);
  this->lineNums.push_back(line);
}

template<typename Number_T>
size_t QueueInputStream<Number_T>::size() { return this->numbers.size(); }

} // namespace wtk
