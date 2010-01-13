// type of automatically assigned ids, mirko.rahn@itwm.fraunhofer.de

#ifndef _HANDLE_HPP
#define _HANDLE_HPP

namespace handle
{
  // Martin Kühn: If you aquire a new handle each cycle, then, with 3e9
  // cycles per second, you can run for 2^64/3e9/60/60/24/365 ~ 195 years.
  // It follows that an uint64_t is enough for now.
  typedef uint64_t T;

  static const T invalid (std::numeric_limits<T>::max());
};

#endif // _HANDLE_HPP
