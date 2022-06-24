// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <drts/certificates.hpp>
#include <drts/client.fwd.hpp>
#include <drts/drts.fwd.hpp>
#include <drts/information_to_reattach.fwd.hpp>
#include <drts/pimpl.hpp>

#include <we/type/value.hpp>

#include <boost/filesystem/path.hpp>

#include <map>
#include <string>
#include <vector>

namespace gspc
{
  class GSPC_DLLEXPORT workflow
  {
  public:
    workflow (::boost::filesystem::path workflow);

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
    client (scoped_runtime_system const&, Certificates const& = ::boost::none);
    explicit client (information_to_reattach const&, Certificates const& = ::boost::none);

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
