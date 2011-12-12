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
#include <sstream>
#include <iomanip>

#include <stdint.h>

namespace bitsetofint
{
  struct type
  {
  public:
    typedef unsigned long element_type;
    typedef std::vector<uint64_t> container_type;
    enum { bits_per_block = 64 };

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

    void ins (const element_type & x)
    {
      if (the_container(x) >= container.size())
        container.resize(the_container(x) + 1, 0);
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

    friend type operator|(const type &, const type &);
    friend std::ostream & operator << (std::ostream &, const type &);
    friend std::size_t hash_value (const type &);
    friend bool operator == (const type &, const type &);
    friend std::string to_hex (const type &);
  };

  inline type operator| (const type & lhs, const type & rhs)
  {
    type result (std::max (lhs.container.size(), rhs.container.size()));

    for (size_t i (0); i < result.container.size(); ++i)
    {
      result.container [i] =
        (
         (i < lhs.container.size() ? lhs.container[i] : 0)
         |
         (i < rhs.container.size() ? rhs.container[i] : 0)
        );
    }

    return result;
  }

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

  inline std::string to_hex (const type & t)
  {
    std::ostringstream oss;

    oss << "0x/";

    for ( type::container_type::const_iterator it (t.container.begin())
        ; it != t.container.end()
        ; ++it
        )
      {
        oss.flags (std::ios::hex);
        oss.width (16);
        oss.fill ('0');
        oss << *it << '/';
      }

    return oss.str();
  }

  inline type from_hex (const std::string & s)
  {
    type::container_type container;

    std::string::const_iterator pos (s.begin());
    const std::string::const_iterator& end (s.end());

    if (  std::distance (pos, end) >= 3
       && *pos == '0' && *(pos+1) == 'x' && *(pos+2) == '/'
       )
    {
      pos += 3;

      while (std::distance (pos, end) >= 17)
      {
        uint64_t value (0);

        std::istringstream iss (std::string (pos, pos + 16));

        iss.flags (std::ios::hex);
        iss.width (16);
        iss.fill ('0');

        iss >> value;

        // TODO: if (iss.bad()) throw

        container.push_back (value);

        pos += 17;
      }
    }
    else
    {
      throw std::runtime_error
        ("bitsetofint::from_hex invalid argument: \"" + s + "\"");
    }

    if (pos != end)
    {
      throw std::runtime_error
        ("bitsetofint::from_hex invalid argument: \"" + s + "\"");
    }
    else
    {
      return type (container);
    }
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
