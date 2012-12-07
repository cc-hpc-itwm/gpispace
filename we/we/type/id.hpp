// mirko.rahn@itwm.fraunhofer.de

#ifndef _TYPE_ID_HPP
#define _TYPE_ID_HPP

#include <stdint.h>

namespace petri_net
{
  // Martin Kühn: If you aquire a new handle each cycle, then, with 3e9
  // cycles per second, you can run for 2^64/3e9/60/60/24/365 > 194 years.
  // It follows that an uint64_t is enough for now.
  typedef uint64_t pid_t; // place
  typedef uint64_t tid_t; // transition
  typedef uint64_t eid_t; // edge
  typedef uint64_t rid_t; // port
  typedef int16_t prio_t; // priority

#define INVALID(_type)                          \
  const _type ## _t& _type ## _invalid();

  INVALID(eid);
  INVALID(pid);
  INVALID(prio);

#undef INVALID
}

#endif
