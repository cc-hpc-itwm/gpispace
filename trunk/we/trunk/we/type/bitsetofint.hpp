// mirko.rahn@itwm.fraunhofer.de

#ifndef _BITSETOFINT_HPP
#define _BITSETOFINT_HPP

#include <algorithm>
#include <stdexcept>
#include <vector>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/functional/hash.hpp>

#include <iostream>

#include <stdint.h>

namespace bitsetofint
{
  struct type
  {
  public:
    typedef uint64_t element_type;
    typedef std::vector<unsigned long> container_type;

  private:
    container_type container;

    inline std::size_t the_container (const element_type & x) const
    {
      return (x >> 6);
    }

    inline std::size_t the_slot (const element_type & x) const
    {
      return (x & 63);
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(container);
    }

  public:
    explicit type (const container_type & c) : container (c) {}
    explicit type (const std::size_t n = 0) : container (n) {}

    void ins (const element_type & x) throw (std::bad_alloc)
    {
      if (the_container(x) >= container.size())
        container.resize(the_container(x) + 1);
      container[the_container(x)] |= (1UL << the_slot(x));
    }

    void del (const element_type & x)
    {
      if (the_container(x) < container.size())
        container[the_container(x)] &= ~(1UL << the_slot(x));
    }

    bool is_element (const element_type & x) const
    {
      return (the_container(x) < container.size())
        && ((container[the_container(x)] & (1UL << the_slot(x))) != 0);
    }

    friend std::ostream & operator << (std::ostream &, const type &);
    friend std::size_t hash_value (const type &);
    friend bool operator == (const type &, const type &);
  };

  inline std::ostream & operator << (std::ostream & s, const type & t)
  {
    s << "{";
    for ( type::container_type::const_iterator it (t.container.begin())
        ; it != t.container.end()
        ; ++it
        )
      {
        s << " " << *it;
      }
    return s << "}";
  }

  inline std::size_t hash_value (const type & t)
  {
    boost::hash<type::container_type> h;

    return h(t.container);
  }

  inline bool operator == (const type & x, const type & y)
  {
    return x.container == y.container;
  }
}

#endif
