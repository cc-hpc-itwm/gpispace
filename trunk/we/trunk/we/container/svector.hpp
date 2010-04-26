// set with access to nth element, mirko.rahn@itwm.fraunhofer.de

#ifndef _CONTAINER_SVECTOR_HPP
#define _CONTAINER_SVECTOR_HPP

#include <vector>

#include <algorithm>
#include <iostream>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/random.hpp>

namespace svector
{
  template<typename T>
  struct type
  {
  public:
    typedef typename std::vector<T> vec_type;
    typedef typename vec_type::size_type size_type;
    typedef typename vec_type::const_reference const_reference;

  private:
    typedef typename vec_type::iterator it_type;
    typedef std::pair<it_type,it_type> pit_t;
    typedef typename vec_type::const_iterator const_it;

    vec_type vec;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(vec);
    }

    pit_t lookup (const T & x)
    {
      return std::equal_range (vec.begin(), vec.end(), x);
    }

  public:
    typedef std::pair<const_it,const_it> const_pit_t;

    const_pit_t lookup (const T & x) const
    {
      return std::equal_range (vec.begin(), vec.end(), x);
    }

    template<typename PIT>
    bool member (const PIT & pit) const
    {
      return std::distance (pit.first, pit.second) > 0;
    }

    void insert (const T & x)
    {
      const pit_t pit (lookup (x));

      if (!member (pit))
        vec.insert (pit.second, x);
    }

    void erase (const T & x)
    {
      const pit_t pit (lookup (x));

      if (member (pit))
        vec.erase (pit.first);
    }

    bool elem (const T & x) const
    {
      return member (lookup (x));
    }

    size_type size (void) const
    {
      return vec.size();
    }

    const_reference first (void) const
    {
      return vec.at (0);
    }

    template<typename Engine>
    const_reference random (Engine & engine) const
    {
      boost::uniform_int<size_type> rand (0, vec.size()-1);
      return vec.at (rand (engine));
    }

    bool empty (void) const { return vec.empty(); }

    bool operator == (const type<T> & other) const
    {
      return (vec == other.vec);
    }

    template<typename A>
    friend std::ostream & operator << (std::ostream &, const type<A> &);
  };

  template<typename A>
  std::ostream & operator << (std::ostream & s, const type<A> & v)
  {
    s << "[";
    
    for ( typename type<A>::vec_type::const_iterator pos (v.vec.begin())
        ; pos != v.vec.end()
        ; ++pos
        )
      s << ((pos != v.vec.begin()) ? ", " : "") << *pos;

    return s << "]";
  }
}

#endif
