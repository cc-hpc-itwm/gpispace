// mirko.rahn@itwm.fraunhofer.de

#ifndef DRTS_CLIENT_HPP
#define DRTS_CLIENT_HPP

#include <drts/client.fwd.hpp>
#include <drts/drts.fwd.hpp>

#include <sdpa/client.hpp>

#include <we/type/activity.hpp>
//! \todo eliminate this include (that completes type transition_t::data)
#include <we/type/net.hpp>
#include <we/type/value.hpp>

#include <boost/filesystem.hpp>

#include <map>
#include <string>

namespace gspc
{
  class client
  {
  public:
    client (scoped_runtime_system const&);

    job_id_t submit
      ( boost::filesystem::path const& workflow
      , std::multimap< std::string
                     , pnet::type::value::value_type
                     > const& values_on_ports
      );
    std::multimap<std::string, pnet::type::value::value_type>
      wait_and_extract (job_id_t);

    std::multimap<std::string, pnet::type::value::value_type>
      put_and_run
      ( boost::filesystem::path const& workflow
      , std::multimap< std::string
                     , pnet::type::value::value_type
                     > const& values_on_ports
      )
    {
      return wait_and_extract (submit (workflow, values_on_ports));
    }

    void put_token ( job_id_t
                   , std::string place_name
                   , pnet::type::value::value_type
                   );

  private:
    sdpa::client::Client _client;

    job_id_t submit
      ( we::type::activity_t activity
      , std::multimap< std::string
                     , pnet::type::value::value_type
                     > const& values_on_ports
      );
  };
}

#endif
