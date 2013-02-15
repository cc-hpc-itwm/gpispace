// store with priorities, mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_CONTAINER_PRIOSTORE_HPP
#define _WE_CONTAINER_PRIOSTORE_HPP

#include <we/container/svector.hpp>
#include <we/type/id.hpp>

#include <map>

#include <stdexcept>
#include <iostream>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/unordered_map.hpp>
#include <boost/function.hpp>

namespace priostore
{
  template<typename T>
  struct type
  {
  private:
    typedef std::greater<petri_net::priority_type> Compare;
    typedef std::map<petri_net::priority_type, svector::type<T>, Compare> prio_map_t;
    typedef boost::unordered_map<T, petri_net::priority_type> get_prio_t;

    prio_map_t prio_map;
    get_prio_t get_prio;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(prio_map);
      ar & BOOST_SERIALIZATION_NVP(get_prio);
    }

    void erase (const T & x, const typename prio_map_t::iterator & pos)
    {
      if (pos != prio_map.end())
        {
          pos->second.erase(x);

          if (pos->second.empty())
            prio_map.erase (pos);
        }
    }

    void insert (const T & x, const petri_net::priority_type & prio)
    {
      prio_map[prio].insert(x);
    }

  public:
    petri_net::priority_type get_priority (const T & x) const
    {
      typename get_prio_t::const_iterator pos (get_prio.find (x));

      return (pos == get_prio.end()) ? petri_net::priority_type() : pos->second;
    }

    void set_priority (const T & x, const petri_net::priority_type & prio)
    {
      const bool is_elem (elem (x));

      if (is_elem)
        erase (x);

      get_prio[x] = prio;

      if (is_elem)
        insert (x, prio);
    }

    void erase_priority (const T & x)
    {
      const bool is_elem (elem (x));

      if (elem (x))
        erase (x);

      get_prio.erase (x);

      if (is_elem)
        insert (x);
    }

    void insert (const T & x)
    {
      insert (x, get_priority (x));
    }

    void erase (const T & x)
    {
      erase (x, prio_map.find (get_priority (x)));
    }

    bool elem (const T & x) const
    {
      typename prio_map_t::const_iterator pos
        (prio_map.find (get_priority (x)));

      return (pos != prio_map.end()) ? pos->second.elem (x) : false;
    }

    std::size_t size (void) const
    {
      std::size_t s (0);

      for ( typename prio_map_t::const_iterator pos (prio_map.begin())
          ; pos != prio_map.end()
          ; ++pos
          )
        s += pos->second.size();

      return s;
    }

    typename svector::type<T>::const_reference first (void) const
    {
      return prio_map.begin()->second.first();
    }

    template<typename Engine>
    typename svector::type<T>::const_reference random (Engine & engine) const
    {
      return prio_map.begin()->second.random(engine);
    }

    bool empty (void) const { return prio_map.empty(); }

    bool operator == (const type<T> & other) const
    {
      return (prio_map == other.prio_map);
    }

    template<typename A>
    friend std::ostream & operator << (std::ostream &, const type<A> &);
  };

  template<typename A>
  std::ostream & operator << (std::ostream & s, const type<A> & p)
  {
    for ( typename type<A>::prio_map_t::const_iterator prio (p.prio_map.begin())
        ; prio != p.prio_map.end()
        ; ++prio
        )
      s << "prio = " << prio->first
        << std::endl << prio->second
        << std::endl;

    return s << std::endl;
  }
}

#endif
