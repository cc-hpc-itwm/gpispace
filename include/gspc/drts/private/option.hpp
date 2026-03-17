// Copyright (C) 2014-2015,2018-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <filesystem>
#include <optional>
#include <boost/program_options.hpp>

#if GSPC_WITH_IML
#include <gspc/iml/gaspi/NetdevID.hpp>
#endif

namespace gspc
{
  void set_log_host (::boost::program_options::variables_map&, std::string const&);
  std::optional<std::string> get_log_host (::boost::program_options::variables_map const&);
  std::string require_log_host (::boost::program_options::variables_map const&);

  void set_log_port (::boost::program_options::variables_map&, unsigned short);
  std::optional<unsigned short> get_log_port (::boost::program_options::variables_map const&);
  unsigned short require_log_port (::boost::program_options::variables_map const&);

  void set_log_level (::boost::program_options::variables_map&, std::string const&);
  std::optional<std::string> get_log_level (::boost::program_options::variables_map const&);
  std::string require_log_level (::boost::program_options::variables_map const&);

  void set_log_directory (::boost::program_options::variables_map&, std::filesystem::path const&);
  std::optional<std::filesystem::path> get_log_directory (::boost::program_options::variables_map const&);
  std::filesystem::path require_log_directory (::boost::program_options::variables_map const&);

  std::optional<std::filesystem::path> get_gspc_home (::boost::program_options::variables_map const&);
  std::filesystem::path require_gspc_home (::boost::program_options::variables_map const&);

  void set_nodefile (::boost::program_options::variables_map&, std::filesystem::path const&);
  std::optional<std::filesystem::path> get_nodefile (::boost::program_options::variables_map const&);
  std::filesystem::path require_nodefile (::boost::program_options::variables_map const&);

  std::optional<std::filesystem::path> get_application_search_path (::boost::program_options::variables_map const&);
  std::filesystem::path require_application_search_path (::boost::program_options::variables_map const&);

  void set_agent_port (::boost::program_options::variables_map&, unsigned short);
  std::optional<unsigned short> get_agent_port (::boost::program_options::variables_map const&);
  unsigned short require_agent_port (::boost::program_options::variables_map const&);

  //! Copy the given environment variables with the currently exported
  //! value to the remote workers.
  void set_worker_env_copy_variable (::boost::program_options::variables_map&, std::vector<std::string> const&);
  std::optional<std::vector<std::string>> get_worker_env_copy_variable (::boost::program_options::variables_map const&);
  std::vector<std::string> require_worker_env_copy_variable (::boost::program_options::variables_map const&);

  //! Copy the entire environment from the current process to the remote
  //! worker, with no filter at all.
  //! \note Equivalent to specifying `copy_variable` for all keys in
  //! the environment.
  void set_worker_env_copy_current (::boost::program_options::variables_map&, bool);
  std::optional<bool> get_worker_env_copy_current (::boost::program_options::variables_map const&);
  bool require_worker_env_copy_current (::boost::program_options::variables_map const&);

  //! Copy the environment described in the given files to the remote
  //! worker.
  //! \note If multiple files are given which contain the same key,
  //! the used value is unspecified, but one of the given values.
  //! \note A file shall have `key=value` lines. This implies
  //! multi-line values are not possible. The key is defined as
  //! everything up to the first `=`.
  //! \note Overwrites `copy_variable`s given and anything added by
  //! `copy_current` if given.
  void set_worker_env_copy_file (::boost::program_options::variables_map&, std::vector<std::filesystem::path> const&);
  std::optional<std::vector<std::filesystem::path>> get_worker_env_copy_file (::boost::program_options::variables_map const&);
  std::vector<std::filesystem::path> require_worker_env_copy_file (::boost::program_options::variables_map const&);

  //! Set a given environment variable in the remote worker process.
  //! \note Arguments shall be of format `key=value`.
  //! \note Overwrites all `copy_` arguments.
  //! \todo Let this be a `map<string, string>` for UX?
  void set_worker_env_set_variable (::boost::program_options::variables_map&, std::vector<std::string> const&);
  std::optional<std::vector<std::string>> get_worker_env_set_variable (::boost::program_options::variables_map const&);
  std::vector<std::string> require_worker_env_set_variable (::boost::program_options::variables_map const&);

  #if GSPC_WITH_IML
  std::optional<std::filesystem::path> get_remote_iml_vmem_socket (::boost::program_options::variables_map const&);
  std::filesystem::path require_remote_iml_vmem_socket (::boost::program_options::variables_map const&);

  void set_virtual_memory_socket (::boost::program_options::variables_map&, std::filesystem::path const&);
  std::optional<std::filesystem::path> get_virtual_memory_socket (::boost::program_options::variables_map const&);
  std::filesystem::path require_virtual_memory_socket (::boost::program_options::variables_map const&);

  void set_virtual_memory_port (::boost::program_options::variables_map&, unsigned short);
  std::optional<unsigned short> get_virtual_memory_port (::boost::program_options::variables_map const&);
  unsigned short require_virtual_memory_port (::boost::program_options::variables_map const&);

  void set_virtual_memory_startup_timeout (::boost::program_options::variables_map&, unsigned long const&);
  std::optional<unsigned long> get_virtual_memory_startup_timeout (::boost::program_options::variables_map const&);
  unsigned long require_virtual_memory_startup_timeout (::boost::program_options::variables_map const&);

  void set_virtual_memory_netdev_id (::boost::program_options::variables_map&, gspc::iml::gaspi::NetdevID const&);
  std::optional<gspc::iml::gaspi::NetdevID> get_virtual_memory_netdev_id (::boost::program_options::variables_map const&);
  gspc::iml::gaspi::NetdevID require_virtual_memory_netdev_id (::boost::program_options::variables_map const&);
  #endif

  void set_rif_entry_points_file (::boost::program_options::variables_map&, std::filesystem::path const&);
  std::optional<std::filesystem::path> get_rif_entry_points_file (::boost::program_options::variables_map const&);
  std::filesystem::path require_rif_entry_points_file (::boost::program_options::variables_map const&);

  void set_rif_port (::boost::program_options::variables_map&, unsigned short);
  std::optional<unsigned short> get_rif_port (::boost::program_options::variables_map const&);
  unsigned short require_rif_port (::boost::program_options::variables_map const&);

  void set_rif_strategy (::boost::program_options::variables_map&, std::string const&);
  std::optional<std::string> get_rif_strategy (::boost::program_options::variables_map const&);
  std::string require_rif_strategy (::boost::program_options::variables_map const&);

  std::optional<std::vector<std::string>> get_rif_strategy_parameters (::boost::program_options::variables_map const&);
  std::vector<std::string> require_rif_strategy_parameters (::boost::program_options::variables_map const&);
  char const* name_rif_strategy_parameters();

  // These are used in the testing tools library, so while they aren't
  // public API (and thus aren't in public headers), they still need
  // to be exported.
  // \todo Merge into testing library (splits knowledge), or this
  // header (needs more different macros or inlining of them)?
  GSPC_EXPORT void set_virtual_memory_socket
    ( ::boost::program_options::variables_map&
    , std::filesystem::path const&
    );
  GSPC_EXPORT void set_nodefile
    ( ::boost::program_options::variables_map&
    , std::filesystem::path const&
    );
  GSPC_EXPORT void set_agent_port
    (::boost::program_options::variables_map&, unsigned short);
  GSPC_EXPORT char const* name_rif_strategy_parameters();
}
