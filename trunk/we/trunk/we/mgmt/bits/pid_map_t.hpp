/*
 * =====================================================================================
 *
 *       Filename:  pid_map_t.hpp
 *
 *    Description:  defines a place-id to place-id map
 *
 *        Version:  1.0
 *        Created:  03/29/2010 03:39:14 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_MGMT_BITS_PID_MAP_T_HPP
#define WE_MGMT_BITS_PID_MAP_T_HPP 1

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>

namespace we { namespace mgmt { namespace detail { namespace traits {
  template <typename Pid>
  struct pid_map_traits
  {
    typedef Pid pid_t;
    typedef boost::bimaps::unordered_set_of<pid_t> pid_collection_t;
    typedef boost::bimap<pid_collection_t, pid_collection_t, boost::bimaps::unordered_set_of_relation<> > type;
    typedef std::pair<pid_t, bool> result_type;

    static result_type map_from(type const & map, const pid_t p)
    {
      const typename type::right_map::const_iterator it (map.right.find(p));
      if (it == map.right.end())
      {
        return std::make_pair(p, false);
      }
      else
      {
        return std::make_pair(it->second, true);
      }
    }

    static result_type map_to(type const & map, const pid_t p)
    {
      const typename type::left_map::const_iterator it (map.left.find(p));
      if (it == map.left.end())
      {
        return std::make_pair(p, false);
      }
      else
      {
        return std::make_pair(it->second, true);
      }
    }

    static void map(type & map, const pid_t global, const pid_t local)
    {
      map.insert( type::value_type(global, local) );
    }
  };
}}}}

#endif
