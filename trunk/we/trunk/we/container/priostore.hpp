// store with priorities, mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_CONTAINER_PRIOSTORE_HPP
#define _WE_CONTAINER_PRIOSTORE_HPP

#include <we/container/svector.hpp>
#include <we/type/id.hpp>

#include <map>

#include <stdexcept>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/unordered_map.hpp>
#include <boost/function.hpp>

namespace priostore
{
  template< typename T
          , typename Prio = petri_net::prio_t
          , typename Store = svector::type<T>
          >
  struct type
  {
  private:
    typedef std::greater<Prio> Compare;
    typedef std::map<Prio, Store, Compare> prio_map_t;
    typedef boost::unordered_map<T, Prio> get_prio_t;

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

    void insert (const T & x, const Prio & prio)
    {
      prio_map[prio].insert(x);
    }

  public:
    Prio get_priority (const T & x) const
    {
      typename get_prio_t::const_iterator pos (get_prio.find (x));

      return (pos == get_prio.end()) ? Prio() : pos->second;
    }

    void set_priority (const T & x, const Prio & prio)
    {
      typename get_prio_t::iterator pos (get_prio.find (x));

      if (pos != get_prio.end())
        {
          erase (x, prio_map.find (pos->second));
          insert (x, prio);
          pos->second = prio;
        }
      else
        get_prio[x] = prio;
    }

    void insert (const T & x)
    {
      insert (x, get_priority (x));
    }

    void erase (const T & x)
    {
      typename prio_map_t::iterator pos (prio_map.find (get_priority (x)));

      erase (x, pos);

      get_prio.erase (x);
    }

    bool elem (const T & x) const
    {
      typename prio_map_t::const_iterator pos (prio_map.find (get_priority (x)));

      return (pos != prio_map.end()) ? pos->second.elem (x) : false;
    }

    typename Store::const_reference first (void) const
    {
      return prio_map.begin()->second.first();
    }

    template<typename Engine>
    typename Store::const_reference random (Engine & engine) const
    {
      return prio_map.begin()->second.random(engine);
    }

    bool empty (void) const { return prio_map.empty(); }

    bool operator == (const type<T,Prio,Store> & other) const
    {
      return (prio_map == other.prio_map);
    }
  };
}

#endif
