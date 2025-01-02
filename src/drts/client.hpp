// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <drts/certificates.hpp>
#include <drts/client.fwd.hpp>
#include <drts/drts.fwd.hpp>
#include <drts/information_to_reattach.fwd.hpp>
#include <drts/pimpl.hpp>

#include <we/type/value.hpp>

#include <boost/filesystem/path.hpp>

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace gspc
{
  class GSPC_DLLEXPORT workflow
  {
  public:
    [[deprecated ("use workflow (std::filesystem::path)")]]
    workflow (::boost::filesystem::path workflow);
    workflow (std::filesystem::path workflow);

    void set_wait_for_output();

    std::string to_string() const;

    workflow (workflow const&) = delete;
    workflow& operator= (workflow const&) = delete;

    workflow (workflow&&) noexcept;
    workflow& operator= (workflow&&) noexcept;

  private:
    friend class ::gspc::client;

    PIMPL (workflow);
  };

  class GSPC_DLLEXPORT client
  {
  public:
    //! \note The drts client constructor/destructor is not thread-safe
    // with the scoped_rifd's constructor/destructor. However, such cases, when
    // they are called concurrently, are extremly unlikely to happen in practice.
    client (scoped_runtime_system const&, Certificates const& = {});
    explicit client (information_to_reattach const&, Certificates const& = {});

    client (client const&) = delete;
    client (client&&) = delete;
    client& operator= (client const&) = delete;
    client& operator= (client&&) = delete;

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
      wait_and_extract (job_id_t job_id);

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

    pnet::type::value::value_type synchronous_workflow_response
      (job_id_t, std::string place_name, pnet::type::value::value_type);

    PIMPL (client);
  };
}
