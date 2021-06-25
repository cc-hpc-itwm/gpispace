// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <util-generic/dllexport.hpp>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <iml/gaspi/NetdevID.hpp>

namespace gspc
{
#define SET(_name, _type) \
  void set_ ## _name (boost::program_options::variables_map&, _type const&)
#define GET(_name, _type)                               \
  boost::optional<_type> get_ ## _name                  \
    (boost::program_options::variables_map const&)
#define REQUIRE(_name, _type)                                           \
  _type require_ ## _name (boost::program_options::variables_map const&)
#define ACCESS(_name, _type)                    \
  SET (_name, _type);                           \
  GET (_name, _type);                           \
  REQUIRE (_name, _type)

  ACCESS (log_host, std::string);
  ACCESS (log_port, unsigned short);
  ACCESS (log_level, std::string);

  ACCESS (log_directory, boost::filesystem::path);
  GET (gspc_home, boost::filesystem::path);
  REQUIRE (gspc_home, boost::filesystem::path);
  ACCESS (nodefile, boost::filesystem::path);
  GET (application_search_path, boost::filesystem::path);
  REQUIRE (application_search_path, boost::filesystem::path);
  ACCESS (agent_port, unsigned short);

  //! Copy the given environment variables with the currently exported
  //! value to the remote workers.
  ACCESS (worker_env_copy_variable, std::vector<std::string>);
  //! Copy the entire environment from the current process to the remote
  //! worker, with no filter at all.
  //! \note Equivalent to specifying `copy_variable` for all keys in
  //! the environment.
  ACCESS (worker_env_copy_current, bool);
  //! Copy the environment described in the given files to the remote
  //! worker.
  //! \note If multiple files are given which contain the same key,
  //! the used value is unspecified, but one of the given values.
  //! \note A file shall have `key=value` lines. This implies
  //! multi-line values are not possible. The key is defined as
  //! everything up to the first `=`.
  //! \note Overwrites `copy_variable`s given and anything added by
  //! `copy_current` if given.
  ACCESS (worker_env_copy_file, std::vector<boost::filesystem::path>);
  //! Set a given environment variable in the remote worker process.
  //! \note Arguments shall be of format `key=value`.
  //! \note Overwrites all `copy_` arguments.
  //! \todo Let this be a `map<string, string>` for UX?
  ACCESS (worker_env_set_variable, std::vector<std::string>);

  GET (remote_iml_vmem_socket, boost::filesystem::path);
  REQUIRE (remote_iml_vmem_socket, boost::filesystem::path);

  ACCESS (virtual_memory_socket, boost::filesystem::path);
  ACCESS (virtual_memory_port, unsigned short);
  ACCESS (virtual_memory_startup_timeout, unsigned long);
  ACCESS (virtual_memory_netdev_id, iml::gaspi::NetdevID);

  ACCESS (rif_entry_points_file, boost::filesystem::path);
  ACCESS (rif_port, unsigned short);
  ACCESS (rif_strategy, std::string);

  GET (rif_strategy_parameters, std::vector<std::string>);
  REQUIRE (rif_strategy_parameters, std::vector<std::string>);
  char const* name_rif_strategy_parameters();

  // These are used in the testing tools library, so while they aren't
  // public API (and thus aren't in public headers), they still need
  // to be exported.
  // \todo Merge into testing library (splits knowledge), or this
  // header (needs more different macros or inlining of them)?
  FHG_UTIL_DLLEXPORT void set_virtual_memory_socket
    ( boost::program_options::variables_map&
    , boost::filesystem::path const&
    );
  FHG_UTIL_DLLEXPORT void set_nodefile
    ( boost::program_options::variables_map&
    , boost::filesystem::path const&
    );
  FHG_UTIL_DLLEXPORT void set_agent_port
    (boost::program_options::variables_map&, unsigned short const&);
  FHG_UTIL_DLLEXPORT char const* name_rif_strategy_parameters();

#undef ACCESS
#undef REQUIRE
#undef GET
#undef SET
}
