// mirko.rahn@itwm.fraunhofer.de

#ifndef _BITSETOFINT_HPP
#define _BITSETOFINT_HPP

#include <algorithm>
#include <stdexcept>
#include <vector>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/functional/hash.hpp>

#include <boost/optional.hpp>

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

    type& ins (const element_type & x)
    {
      if (the_container(x) >= container.size())
        container.resize(the_container(x) + 1, 0);
      container[the_container(x)] |= (1UL << the_slot(x));

      return *this;
    }

    type& del (const element_type & x)
    {
      if (the_container(x) < container.size())
        container[the_container(x)] &= ~(1UL << the_slot(x));

      return *this;
    }

    bool is_element (const element_type & x) const
    {
      return (the_container(x) < container.size())
        && ((container[the_container(x)] & (1UL << the_slot(x))) != 0);
    }

    std::size_t count () const
    {
      std::size_t cnt = 0;

      for (size_t c = 0; c < container.size(); ++c)
      {
        element_type block = container[c];
        for (size_t bit = 0 ; bit < bits_per_block ; ++bit)
        {
          cnt += (block >> bit) & 0x1;
        }
      }

      return cnt;
    }

    void list (std::ostream& s) const
    {
      size_t x (0);

      for (size_t c (0); c < container.size(); ++c)
        {
          element_type block (container[c]);

          for (size_t bit (0); bit < bits_per_block; ++x, ++bit, block >>= 1)
            {
              if (block & 0x1)
                {
                  s << x << std::endl;
                }
            }
        }
    }

    friend type operator | (const type &, const type &);
    friend type operator & (const type &, const type &);
    friend type operator ^ (const type &, const type &);

    friend std::ostream & operator << (std::ostream &, const type &);
    friend std::size_t hash_value (const type &);
    friend bool operator == (const type &, const type &);
    friend std::string to_hex (const type &);
    template<typename IT> friend std::string to_hex (IT&, const IT&);
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

  inline type operator& (const type & lhs, const type & rhs)
  {
    type result (std::max (lhs.container.size(), rhs.container.size()));

    for (size_t i (0); i < result.container.size(); ++i)
    {
      result.container [i] =
        (
         (i < lhs.container.size() ? lhs.container[i] : 0)
         &
         (i < rhs.container.size() ? rhs.container[i] : 0)
        );
    }

    return result;
  }

  inline type operator^ (const type & lhs, const type & rhs)
  {
    type result (std::max (lhs.container.size(), rhs.container.size()));

    for (size_t i (0); i < result.container.size(); ++i)
    {
      result.container [i] =
        (
         (i < lhs.container.size() ? lhs.container[i] : 0)
         ^
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

  template<typename IT>
  inline boost::optional<type> from_hex (IT& pos, IT const &end)
  {
    if (  (pos + 0) != end && *(pos + 0) == '0'
       && (pos + 1) != end && *(pos + 1) == 'x'
       )
      {
        std::advance (pos, 2);

        type::container_type container;

        while (pos != end && *pos == '/')
          {
            std::advance (pos, 1);

            if (std::distance (pos, end) >= 16)
              {
                uint64_t value (0);

                std::istringstream iss (std::string (pos, pos + 16));

                iss.flags (std::ios::hex);
                iss.width (16);
                iss.fill ('0');

                iss >> value;

                if (iss.fail() && !iss.eof())
                  {
                    return type (container);
                  }

                container.push_back (value);

                std::advance (pos, 16);
              }
          }

        return type (container);
      }

    return boost::none;
  }

  inline type from_hex (const std::string & s)
  {
    std::string::const_iterator pos (s.begin());
    const std::string::const_iterator& end (s.end());

    boost::optional<type> mtype (from_hex (pos, end));

    if (!mtype)
    {
      throw std::runtime_error
        ("bitsetofint::from_hex: missing prefix 0x");
    }

    if (pos != end)
    {
      throw std::runtime_error
        ("bitsetofint::from_hex invalid argument: \"" + s + "\""
        + ", rest after parsing: " + std::string (pos, end)
        );
    }

    return *mtype;
  }

  inline std::size_t hash_value (const type & t)
  {
    boost::hash<type::container_type> h;

    return h(t.container);
  }

  inline bool operator == (const type & x, const type & y)
  {
    type::container_type::const_iterator pos_x (x.container.begin());
    const type::container_type::const_iterator& end_x (x.container.end());
    type::container_type::const_iterator pos_y (y.container.begin());
    const type::container_type::const_iterator& end_y (y.container.end());

    while (pos_x != end_x && pos_y != end_y)
      {
        if (*pos_x != *pos_y)
          {
            return false;
          }

        ++pos_x;
        ++pos_y;
      }

    while (pos_x != end_x)
      {
        if (*pos_x != 0)
          {
            return false;
          }

        ++pos_x;
      }

    while (pos_y != end_y)
      {
        if (*pos_y != 0)
          {
            return false;
          }

        ++pos_y;
      }

    return true;
  }
}

#endif
