// set with access to nth element, mirko.rahn@itwm.fraunhofer.de

#ifndef _CONTAINER_SVECTOR_HPP
#define _CONTAINER_SVECTOR_HPP

#include <vector>

#include <algorithm>
#include <iostream>

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

    pit_t lookup (const T & x)
    {
      return std::equal_range (vec.begin(), vec.end(), x);
    }

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

  public:
    void insert (const T& x)
    {
      const pit_t pit (lookup (x));

      if (!member (pit))
        vec.insert (pit.second, x);
    }

    void erase (const T& x)
    {
      const pit_t pit (lookup (x));

      if (member (pit))
        vec.erase (pit.first);
    }

    bool elem (const T& x) const
    {
      return member (lookup (x));
    }

    template<typename Engine>
    const_reference random (Engine& engine) const
    {
      boost::uniform_int<size_type> rand (0, vec.size()-1);
      return vec.at (rand (engine));
    }

    bool empty() const
    {
      return vec.empty();
    }
  };
}

#endif
