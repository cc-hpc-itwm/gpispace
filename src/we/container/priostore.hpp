// store with priorities, mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_CONTAINER_PRIOSTORE_HPP
#define _WE_CONTAINER_PRIOSTORE_HPP

#include <we/type/id.hpp>

#include <boost/unordered_map.hpp>
#include <boost/random.hpp>

#include <map>
#include <vector>

namespace we
{
  namespace container
  {
    struct priority_store
    {
    public:
      we::priority_type
      get_priority (const we::transition_id_type&) const;

      void set_priority ( const we::transition_id_type&
                        , const we::priority_type&
                        );

      void insert (const we::transition_id_type&);
      void erase (const we::transition_id_type&);

      bool empty() const;
      bool elem (const we::transition_id_type&) const;

      template<typename Engine>
      const we::transition_id_type& random (Engine& engine) const
      {
        const std::vector<we::transition_id_type>& v
          (_prio_map.begin()->second);
        boost::uniform_int<std::size_t> rand (0, v.size()-1);

        return v.at (rand (engine));
      }

    private:
      typedef std::map< we::priority_type
                      , std::vector<we::transition_id_type>
                      , std::greater<we::priority_type>
                      > prio_map_t;
      typedef boost::unordered_map< we::transition_id_type
                                  , we::priority_type
                                  > get_prio_t;

      prio_map_t _prio_map;
      get_prio_t _get_prio;

      void erase ( const we::transition_id_type&
                 , const prio_map_t::iterator&
                 );
      void insert ( const we::transition_id_type&
                  , const we::priority_type&
                  );
    };
  }
}

#endif
