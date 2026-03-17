// Copyright (C) 2014-2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/drts/information_to_reattach.hpp>
#include <gspc/drts/private/information_to_reattach.hpp>

#include <gspc/drts/client.hpp>
#include <gspc/drts/drts.hpp>
#include <gspc/drts/private/drts_impl.hpp>
#include <gspc/drts/private/pimpl.hpp>

#include <gspc/we/type/value.hpp>
#include <gspc/we/type/value/path/join.hpp>
#include <gspc/we/type/value/peek.hpp>
#include <gspc/we/type/value/poke.hpp>
#include <gspc/we/type/value/read.hpp>
#include <gspc/we/type/value/serialize.hpp>

#include <list>
#include <utility>

namespace gspc
{
  namespace
  {
    template <typename T>
    T const& get_or_throw (std::list<std::string> const& path, gspc::pnet::type::value::value_type const& value)
    {
      std::optional<std::reference_wrapper<gspc::pnet::type::value::value_type const>> const v
        (gspc::pnet::type::value::peek (path, value));
      if (v)
      {
        try
        {
          return ::boost::get<T> (v->get());
        }
        catch (::boost::bad_get const&)
        {
          throw std::logic_error
            ( "value did not had correct type at path: "
            + gspc::pnet::type::value::path::join (path)
            + ": expected type " + typeid (T).name()
            );
        }
      }
      else
      {
        throw std::logic_error
          ( "value did not contain anything at path: "
          + gspc::pnet::type::value::path::join (path)
          );
      }
    }

    gspc::host_and_port_type get_top_level_agent_endpoint
      (gspc::pnet::type::value::value_type const& value)
    {
      return { get_or_throw<std::string> ({"top_level_agent", "host"}, value)
             , static_cast<unsigned short> (get_or_throw<unsigned int> ({"top_level_agent", "port"}, value))
             };
    }
  }

  information_to_reattach::implementation::implementation (std::string const& serialized_value)
    : _endpoint (get_top_level_agent_endpoint (gspc::pnet::type::value::from_string (serialized_value)))
  {}

  information_to_reattach::implementation::implementation (host_and_port_type const& top_level_agent)
    : _endpoint (top_level_agent)
  {}

  std::string information_to_reattach::implementation::to_string () const
  {
    gspc::pnet::type::value::value_type serialized;
    gspc::pnet::type::value::poke ( std::list<std::string> {"top_level_agent", "host"}
                            , serialized
                            , _endpoint.host
                            );
    gspc::pnet::type::value::poke ( std::list<std::string> {"top_level_agent", "port"}
                            , serialized
                            , static_cast<unsigned int> (_endpoint.port)
                            );
    return gspc::pnet::type::value::to_string (serialized);
  }

  gspc::host_and_port_type const&
  information_to_reattach::implementation::endpoint() const
  {
    return _endpoint;
  }

  information_to_reattach::information_to_reattach (scoped_runtime_system const& drts)
    : _ (new implementation
          ( gspc::host_and_port_type
              { drts._->_started_runtime_system._top_level_agent_host
              , drts._->_started_runtime_system._top_level_agent_port
              }
          )
        )
  {}
  information_to_reattach::information_to_reattach (std::string const& drts_info)
    : _ (new implementation (drts_info))
  {}
  PIMPL_DTOR (information_to_reattach)
  std::string information_to_reattach::to_string () const
  {
    return _->to_string();
  }
}
