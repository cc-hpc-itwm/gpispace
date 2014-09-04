#include <drts/vmem.hpp>

#include <fhg/util/boost/program_options/validators/executable.hpp>
#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/boost/program_options/validators/is_directory_if_exists.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <fhg/util/boost/program_options/validators/nonexisting_path.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/make_unique.hpp>

namespace gspc
{
  namespace validators = fhg::util::boost::program_options;

  vmem_t::vmem_t ( boost::program_options::variables_map const& vm
                 , gspc::installation const& installation
                 , gspc::rif_t& rif
                 , std::pair<std::list<std::string>, unsigned long> const& machinefile
                 , const std::string& kvs_host
                 , const unsigned short kvs_port
                 )
    : _machinefile (machinefile)
    , _rif (rif)
  {
    const std::string& log_host
      (vm[options::name::log_host].as<validators::nonempty_string>());
    const unsigned short log_port
      (vm[options::name::log_port].as<validators::positive_integral<unsigned short>>());
    const std::string& log_level
      (vm[options::name::log_level].as<std::string>());
    const boost::filesystem::path socket
      (vm[options::name::virtual_memory_socket].as<validators::nonexisting_path>());
    const unsigned short port
      (vm[options::name::virtual_memory_port].as<validators::positive_integral<unsigned short>>());
    const unsigned long memory_size
      (vm[options::name::virtual_memory_per_node].as<validators::positive_integral<unsigned long>>());
    const unsigned long timeout
      (vm[options::name::virtual_memory_timeout].as<validators::positive_integral<unsigned long>>());

    if (0 == _machinefile.second)
    {
      throw std::runtime_error ("at least one node is required in machinefile");
    }

    if (  _machinefile.first.size()
       != _machinefile.second
       )
    {
      throw std::runtime_error ("each host has to appear just once in nodefile");
    }

    const validators::executable vmem_binary
      ((installation.gspc_home() / "bin" / "gpi-space").string());

    if (boost::filesystem::exists (socket))
    {
      throw std::runtime_error ("socket file already exists: " + socket.string());
    }

    const std::string& master (_machinefile.first.front());
    const std::list<gspc::rif_t::endpoint_t> rif_endpoints
      ([] (const std::list<std::string>& hosts) -> std::list<gspc::rif_t::endpoint_t> {
        std::list<gspc::rif_t::endpoint_t> ep;
        for (std::string const&h : hosts)
        {
          ep.emplace_back (gspc::rif_t::endpoint_t (h, 22));
        }
        return std::move (ep);
      }(_machinefile.first));

    // create in memory hostlist
    // rif.store (machinefile, hostlist, "<rif-root>/machinefile");

    const std::string machinefile_path
      (vm[options::name::nodefile].as<validators::existing_path>().string());

    _rif.exec ( {rif_endpoints.front()}
              , "gaspi-master"
              , { vmem_binary.string()
                , "--kvs-host", kvs_host, "--kvs-port", std::to_string (kvs_port)
                , "--log-host", log_host, "--log-port", std::to_string (log_port)
                , "--log-level", log_level
                , "--gpi-mem", std::to_string (memory_size)
                , "--socket", socket.string()
                , "--port", std::to_string (port)
                , "--gpi-api", _machinefile.first.size() > 1 ? "gaspi" : "fake"
                , "--gpi-timeout", std::to_string (timeout)
                }
              , { {"GASPI_MFILE", machinefile_path}
                , {"GASPI_MASTER", master}
                , {"GASPI_SOCKET", "0"}
                , {"GASPI_TYPE", "GASPI_MASTER"}
                , {"GASPI_SET_NUMA_SOCKET", "0"}
                }
              );

    if (_machinefile.first.size() > 1)
    {
      _rif.exec ( std::list<gspc::rif_t::endpoint_t> ( ++rif_endpoints.begin()
                                                     , rif_endpoints.end()
                                                     )
                , "gaspi-worker"
                , { vmem_binary.string()
                  , "--kvs-host", kvs_host, "--kvs-port", std::to_string (kvs_port)
                  , "--log-host", log_host, "--log-port", std::to_string (log_port)
                  , "--log-level", log_level
                  , "--gpi-mem", std::to_string (memory_size)
                  , "--socket", socket.string()
                  , "--port", std::to_string (port)
                  , "--gpi-api", _machinefile.first.size() > 1 ? "gaspi" : "fake"
                  , "--gpi-timeout", std::to_string (timeout)
                  }
                , { {"GASPI_MFILE", machinefile_path}
                  , {"GASPI_MASTER", master}
                  , {"GASPI_SOCKET", "0"}
                  , {"GASPI_TYPE", "GASPI_WORKER"}
                  , {"GASPI_SET_NUMA_SOCKET", "0"}
                  }
                );
    }

    std::uint64_t slept (0);
    while (!boost::filesystem::exists (socket))
    {
      const std::uint64_t sleep_duration (1000);

      usleep (sleep_duration * 1000);
      slept += sleep_duration;

      if (slept > timeout)
      {
        throw std::runtime_error
          ( "timeout while waiting for: " + socket.string()
          + " waited: " + std::to_string (slept) + " seconds"
          );
      }
    }
  }

  vmem_t::~vmem_t()
  {
    // replace with actual list of rifs
    if (_machinefile.first.size() > 1)
    {
      _rif.stop ({}, "gaspi-worker");
    }

    _rif.stop ({}, "gaspi-master");
  }
}
