// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_UTIL_CROSS_HPP
#define _WE_UTIL_CROSS_HPP

#include <we/util/cross.fwd.hpp>

#include <we/type/id.hpp>

#include <we/type/transition.fwd.hpp>

#include <list>

#include <boost/unordered_map.hpp>

namespace we
{
  namespace util
  {
    class iterators_type
    {
    public:
      iterators_type (std::list<value::type>&);
      iterators_type (const std::list<value::type>::iterator&);
      bool end() const;
      const pos_and_distance_type& pos_and_distance() const;
      void operator++();
      void rewind();
    private:
      std::list<value::type>::iterator _begin;
      std::list<value::type>::iterator _end;
      pos_and_distance_type _pos_and_distance;
    };

    class cross_type
    {
    public:
      bool empty() const;
      bool step();
      bool eval (const we::type::transition_t&) const;
      void write_to (boost::unordered_map< petri_net::place_id_type
                                         , pos_and_distance_type
                                         >&
                    ) const;
      void write_to (boost::unordered_map< petri_net::place_id_type
                                         , std::list<value::type>::iterator
                                         >&
                    ) const;
      void write_to (boost::unordered_map< petri_net::place_id_type
                                         , value::type
                                         >&
                    ) const;
      void push ( const petri_net::place_id_type&
                , std::list<value::type>&
                );
      void push ( const petri_net::place_id_type&
                , const std::list<value::type>::iterator&
                );
    private:
      boost::unordered_map<petri_net::place_id_type, iterators_type> _m;
    };
  }
}

#endif
