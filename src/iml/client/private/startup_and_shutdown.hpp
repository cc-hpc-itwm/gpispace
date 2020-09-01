#pragma once

#include <iml/client/iml.fwd.hpp>

#include <iml/installation_path.hpp>

#include <iml/rif/entry_point.hpp>
#include <iml/rif/protocol.hpp>

#include <iml/vmem/netdev_id.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fhg
{
  namespace iml_drts
  {
    enum class component_type
    {
      vmem,
    };

    struct processes_storage : boost::noncopyable
    {
      std::mutex _guard;
      std::unordered_map < fhg::iml::rif::entry_point
                         , std::unordered_map<std::string /*name*/, pid_t>
                         > _;

      processes_storage (std::ostream& info_output)
        : _info_output (info_output)
      {}

      ~processes_storage();

      void store (fhg::iml::rif::entry_point const&, std::string const& name, pid_t);
      boost::optional<pid_t> pidof
        (fhg::iml::rif::entry_point const&, std::string const& name);

    private:
      std::ostream& _info_output;
    };

    void startup
      ( boost::optional<boost::filesystem::path> gpi_socket
      , iml_client::installation_path const&
      , processes_storage& processes
      , boost::optional<std::chrono::seconds> vmem_startup_timeout
      , boost::optional<unsigned short> vmem_port
      , boost::optional<iml::vmem::netdev_id> vmem_netdev_id
      , std::vector<fhg::iml::rif::entry_point> const&
      , fhg::iml::rif::entry_point const&
      , std::ostream& info_output
      );
  }
}
