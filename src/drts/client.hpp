// mirko.rahn@itwm.fraunhofer.de

#ifndef DRTS_CLIENT_HPP
#define DRTS_CLIENT_HPP

#include <drts/client.fwd.hpp>
#include <drts/drts.fwd.hpp>
#include <drts/information_to_reattach.fwd.hpp>

#include <we/type/value.hpp>

#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>

#include <map>
#include <string>

namespace gspc
{
  class workflow : boost::noncopyable
  {
  public:
    workflow (boost::filesystem::path workflow);
    ~workflow();

    void set_wait_for_output();

  private:
    struct implementation;
    friend class client;
    implementation* _;
  };

  class client : boost::noncopyable
  {
  public:
    client (scoped_runtime_system const&);
    explicit client (information_to_reattach const&);
    ~client();

    job_id_t submit
      ( workflow const&
      , std::multimap< std::string
                     , pnet::type::value::value_type
                     > const& values_on_ports
      );
    std::multimap<std::string, pnet::type::value::value_type>
      wait_and_extract (job_id_t);

    std::multimap<std::string, pnet::type::value::value_type>
      put_and_run
      ( class workflow const& workflow
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
    struct implementation;
    implementation* _;
  };
}

#endif
