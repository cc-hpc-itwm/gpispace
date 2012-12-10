// mirko.rahn@itwm.fraunhofer.de

#ifndef _TYPE_ID_HPP
#define _TYPE_ID_HPP

#include <stdint.h>

#include <iostream>

#include <boost/serialization/nvp.hpp>
#include <boost/functional/hash.hpp>

namespace petri_net
{
  // Martin Kühn: If you aquire a new handle each cycle, then, with 3e9
  // cycles per second, you can run for 2^64/3e9/60/60/24/365 > 194 years.
  // It follows that an uint64_t is enough for now.

  struct pid_t
  {
    pid_t () : _value (0) {}
    pid_t (const uint64_t& value) : _value (value) {}

    bool operator== (const pid_t& other) const
    {
      return _value == other._value;
    }
    bool operator!= (const pid_t& other) const
    {
      return _value != other._value;
    }
    bool operator< (const pid_t& other) const
    {
      return _value < other._value;
    }

    const pid_t& operator++ ()
    {
      ++_value;

      return *this;
    }

    pid_t operator++ (int)
    {
      pid_t old (*this);

      ++_value;

      return old;
    }

    //! \todo eliminate this
    const std::size_t& value () const
    {
      return _value;
    }

    friend std::ostream& operator<< (std::ostream&, const pid_t&);
    friend std::istream& operator>> (std::istream&, pid_t&);
    friend std::size_t hash_value (const pid_t&);

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive& ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(_value);
    }

  private:
    uint64_t _value;
  };

  //  typedef uint64_t pid_t; // place
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
