// store with priorities, mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_CONTAINER_PRIOSTORE_HPP
#define _WE_CONTAINER_PRIOSTORE_HPP

#include <we/container/svector.hpp>
#include <we/type/id.hpp>

#include <map>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/unordered_map.hpp>

namespace we
{
  namespace container
  {
    struct priority_store
    {
    public:
      petri_net::priority_type
      get_priority (const petri_net::transition_id_type&) const;

      void set_priority ( const petri_net::transition_id_type&
                        , const petri_net::priority_type&
                        );
      void erase_priority (const petri_net::transition_id_type&);

      void insert (const petri_net::transition_id_type&);
      void erase (const petri_net::transition_id_type&);

      bool empty() const;
      bool elem (const petri_net::transition_id_type&) const;

      template<typename Engine>
      svector::type<petri_net::transition_id_type>::const_reference
      random (Engine& engine) const
      {
        return _prio_map.begin()->second.random (engine);
      }

    private:
      typedef std::map< petri_net::priority_type
                      , svector::type<petri_net::transition_id_type>
                      , std::greater<petri_net::priority_type>
                      > prio_map_t;
      typedef boost::unordered_map< petri_net::transition_id_type
                                  , petri_net::priority_type
                                  > get_prio_t;

      prio_map_t _prio_map;
      get_prio_t _get_prio;

      friend class boost::serialization::access;
      template<typename Archive>
      void serialize (Archive & ar, const unsigned int)
      {
        ar & BOOST_SERIALIZATION_NVP (_prio_map);
        ar & BOOST_SERIALIZATION_NVP (_get_prio);
      }

      void erase ( const petri_net::transition_id_type&
                 , const prio_map_t::iterator&
                 );
      void insert ( const petri_net::transition_id_type&
                  , const petri_net::priority_type&
                  );
    };
  }
}

#endif
