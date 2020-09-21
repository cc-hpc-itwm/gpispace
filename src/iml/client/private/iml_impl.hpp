#pragma once

#include <iml/client/iml.hpp>
#include <iml/client/private/startup_and_shutdown.hpp>
#include <iml/vmem/gaspi/pc/client/api.hpp>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include <algorithm>
#include <exception>
#include <list>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <chrono>

namespace iml_client
{
  struct scoped_iml_runtime_system::implementation
  {
    implementation ( boost::program_options::variables_map const& vm
                   , installation const&
                   , boost::optional<rifd_entry_points> const& entry_points
                   , rifd_entry_point const& master
                   , std::ostream& info_output
                   );

    boost::optional<boost::filesystem::path> _virtual_memory_socket;
    boost::optional<std::chrono::seconds> _virtual_memory_startup_timeout;

    struct started_runtime_system
    {
      started_runtime_system ( boost::optional<boost::filesystem::path> gpi_socket
                             , installation_path
                             , boost::optional<std::chrono::seconds> vmem_startup_timeout
                             , boost::optional<unsigned short> vmem_port
                             , boost::optional<fhg::iml::vmem::netdev_id> vmem_netdev_id
                             , std::vector<fhg::iml::rif::entry_point> const& rif_entry_points
                             , fhg::iml::rif::entry_point const& master
                             , std::ostream& info_output
                             );

      fhg::iml::rif::entry_point _master;
      std::ostream& _info_output;
      boost::optional<boost::filesystem::path> _gpi_socket;
      installation_path _installation_path;

      fhg::iml_drts::processes_storage _processes_storage;
    } _started_runtime_system;
    std::unique_ptr<gpi::pc::client::api_t> _virtual_memory_api;
  };
}
