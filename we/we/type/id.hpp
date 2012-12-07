// mirko.rahn@itwm.fraunhofer.de

#ifndef _TYPE_ID_HPP
#define _TYPE_ID_HPP

#include <stdint.h>

#include <limits>

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

  namespace traits
  {
    template <typename IdType>
    struct id_traits
    {
      typedef IdType type;

      inline static type invalid()
      {
        return std::numeric_limits<type>::max();
      }
    };
  }

  // deprecated?
  static const eid_t eid_invalid (traits::id_traits<eid_t>::invalid());
}

#endif
