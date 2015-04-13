#pragma once

#include <we/type/activity.hpp>
#include <we/type/net.hpp>
#include <we/type/property.hpp>
#include <we/type/transition.hpp>

#include <boost/optional.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace we
{
  namespace type
  {
    std::tuple< net_type
              , std::unordered_map<place_id_type, place_id_type>
              , place_id_type
              >
      copy ( net_type const&
           , boost::optional<std::string> control
           , boost::optional<std::vector<std::string>> break_after_transition
           , we::type::property::type transformation_properties
           );

    activity_t
      copy ( activity_t const&
           , boost::optional<std::string> control
           , boost::optional<std::vector<std::string>> break_after_trans
           );

    std::tuple< transition_t
              , std::set<port_id_type>
              , std::unordered_map<port_id_type, port_id_type>
              >
      copy_rev (transition_t const&);

    std::tuple< net_type
              , std::unordered_map<place_id_type, place_id_type>
              , place_id_type
              >
      copy_rev (net_type const&, boost::optional<std::string> control);

    activity_t
      copy_rev (activity_t const&, boost::optional<std::string> control);
  }
}
