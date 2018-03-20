#pragma once

#include <drts/client.fwd.hpp>
#include <drts/drts.fwd.hpp>
#include <drts/information_to_reattach.fwd.hpp>
#include <drts/pimpl.hpp>

#include <we/type/value.hpp>

#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <map>
#include <string>
#include <vector>

namespace gspc
{
  using certificates_t = boost::optional<boost::filesystem::path>;

  class workflow : boost::noncopyable
  {
  public:
    workflow (boost::filesystem::path workflow);

    void set_wait_for_output();

    void add_input ( std::string const& port
                   , pnet::type::value::value_type const& value
                   );

    std::string to_string() const;

    workflow (workflow&&);

  private:
    friend class ::gspc::client;

    PIMPL (workflow);
  };

  class client : boost::noncopyable
  {
  public:
    client (scoped_runtime_system const&, certificates_t const&);
    client (scoped_runtime_system const&);
    explicit client (information_to_reattach const&m, certificates_t const&);

    job_id_t submit
      ( workflow const&
      , std::multimap< std::string
                     , pnet::type::value::value_type
                     > const& values_on_ports
      );
    void wait (job_id_t) const;
    void cancel (job_id_t) const;
    std::multimap<std::string, pnet::type::value::value_type>
      extract_result_and_forget_job (job_id_t);
    std::multimap<std::string, pnet::type::value::value_type>
      wait_and_extract (job_id_t job_id)
    {
      wait (job_id);

      return extract_result_and_forget_job (job_id);
    }

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

    void step (class workflow& workflow, unsigned long number_of_steps);
    void break_after ( class workflow& workflow
                     , std::vector<std::string> transition_names
                     );

    void put_token ( job_id_t
                   , std::string place_name
                   , pnet::type::value::value_type
                   );

    pnet::type::value::value_type synchronous_workflow_response
      (job_id_t, std::string place_name, pnet::type::value::value_type);

    PIMPL (client);
  };
}
