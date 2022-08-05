/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace utils {

template<typename T, size_t batch_size>
T* Pool<T, batch_size>::makeSpace(size_t n)
{
  // TODO handle n > batch_size, also potential wast when n>1

  if(UNLIKELY(
       this->spaces.size() == 0
    || this->spaces.back().second + n >= batch_size))
  {
    this->spaces.emplace_back((T*) malloc(batch_size * sizeof(T)), 0);
  }

  size_t rplace = this->spaces.back().second;
  this->spaces.back().second += n;
  return this->spaces.back().first + rplace;
}

template<typename T, size_t batch_size>
T* Pool<T, batch_size>::allocate(size_t n)
{
  T* space = this->makeSpace(n);

  for(size_t i = 0; i < n; i++)
  {
    new(space + i) T();
  }

  return space;
}

template<typename T, size_t batch_size>
template<typename... Args>
T* Pool<T, batch_size>::allocate(size_t n, Args&&... args)
{
  T* space = this->makeSpace(n);

  for(size_t i = 0; i < n; i++)
  {
    new(space + i) T(std::forward<Args>(args)...);
  }

  return space;
}

template<typename T, size_t batch_size>
Pool<T, batch_size>::~Pool()
{
  for(size_t i = 0; i < this->spaces.size(); i++)
  {
    for(size_t j = 0; j < this->spaces[i].second; j++)
    {
      (this->spaces[i].first + j)->~T();
    }
    free(this->spaces[i].first);
  }
}

} } // namespace wtk::utils
