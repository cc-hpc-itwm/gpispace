// set with access to nth element, mirko.rahn@itwm.fraunhofer.de

#ifndef _CONTAINER_SVECTOR_HPP
#define _CONTAINER_SVECTOR_HPP

#include <vector>

#include <algorithm>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/random.hpp>

namespace svector
{
  template<typename T>
  struct svector
  {
  private:
    typedef typename std::vector<T> vec_t;
    typedef typename vec_t::iterator it;
    typedef std::pair<it,it> pit_t;
    typedef typename vec_t::const_iterator const_it;
    typedef std::pair<const_it,const_it> const_pit_t;

    vec_t vec;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(vec);
    }

  public:
    typedef typename vec_t::const_iterator const_iterator;
    typedef typename vec_t::size_type size_type;

    it insert (const T & x)
    {
      const pit_t pit (std::equal_range (vec.begin(), vec.end(), x));

      return (std::distance (pit.first, pit.second) == 0)
        ? vec.insert (pit.second, x) : pit.first;
    }

    it erase (const T & x)
    {
      const pit_t pit (std::equal_range (vec.begin(), vec.end(), x));

      return (std::distance (pit.first, pit.second) == 0)
        ? pit.first : vec.erase (pit.first);
    }

    bool elem (const T & x) const
    {
      const const_pit_t pit (std::equal_range (vec.begin(), vec.end(), x));

      return (std::distance (pit.first, pit.second) != 0);
    }

    typename vec_t::const_reference & at (typename vec_t::size_type n) const
    {
      return vec.at (n);
    }

    typename vec_t::const_reference & first (void) const
    {
      return at(0);
    }

    template<typename Engine>
    typename vec_t::const_reference & random (Engine & engine) const
    {
      boost::uniform_int<size_type> rand (0, size()-1);
      return vec.at (rand (engine));
    }

    const_iterator begin (void) const { return vec.begin(); }
    const_iterator end (void) const { return vec.end(); }

    bool empty (void) const { return vec.empty(); }
    size_type size (void) const { return vec.size(); }

    bool operator == (const svector<T> & other) const
    {
      return (vec == other.vec);
    }
  };
}

#endif
