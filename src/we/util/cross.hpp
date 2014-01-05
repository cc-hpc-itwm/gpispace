// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_UTIL_CROSS_HPP
#define _WE_UTIL_CROSS_HPP

#include <we/type/condition.hpp>
#include <we/type/id.hpp>
#include <we/type/transition.fwd.hpp>
#include <we/type/value.hpp>

#include <boost/unordered_map.hpp>

#include <list>

namespace we
{
  namespace util
  {
    typedef std::pair< std::list<pnet::type::value::value_type>::iterator
                     , std::size_t
                     > pos_and_distance_type;

    class cross_type
    {
    public:
      bool enables ( we::type::transition_t const&
                   , condition::type const&
                   );
      void write_to (boost::unordered_map< petri_net::place_id_type
                                         , pos_and_distance_type
                                         >&
                    ) const;
      void push ( const petri_net::place_id_type&
                , std::list<pnet::type::value::value_type>&
                );
      void push ( const petri_net::place_id_type&
                , const std::list<pnet::type::value::value_type>::iterator&
                );
    private:
      class iterators_type
      {
      public:
        iterators_type (std::list<pnet::type::value::value_type>&);
        iterators_type (const std::list<pnet::type::value::value_type>::iterator&);
        bool end() const;
        const pos_and_distance_type& pos_and_distance() const;
        void operator++();
        void rewind();
      private:
        std::list<pnet::type::value::value_type>::iterator _begin;
        std::list<pnet::type::value::value_type>::iterator _end;
        pos_and_distance_type _pos_and_distance;
      };

      typedef boost::unordered_map<petri_net::place_id_type, iterators_type>
        map_type;

      map_type _m;

      bool do_step (map_type::iterator, map_type::iterator const&);
    };
  }
}

#endif
