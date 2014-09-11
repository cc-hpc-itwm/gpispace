#include <drts/vmem.hpp>
#include <drts/rif.hpp>

#include <fhg/util/boost/program_options/validators/executable.hpp>
#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/boost/program_options/validators/is_directory_if_exists.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <fhg/util/boost/program_options/validators/nonexisting_path.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/make_unique.hpp>

#include <unordered_set>

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
    : _rif (rif)
    , _rif_endpoints ([] (const std::list<std::string>& hosts) -> std::list<gspc::rif_t::endpoint_t> {
        std::list<gspc::rif_t::endpoint_t> ep;
        std::unordered_set<std::string> seen;
        for (std::string const&h : hosts)
        {
          if (seen.find (h) == seen.end())
          {
            ep.emplace_back (h, 22);
            seen.insert (h);
          }
        }
        return ep;
      } (machinefile.first))
  {
    const std::string& log_host (get_log_host (vm));
    const unsigned short log_port (get_log_port (vm));
    const std::string log_level (get_log_level (vm));
    const boost::filesystem::path socket (get_not_yet_existing_virtual_memory_socket (vm));
    const unsigned short port (get_virtual_memory_port (vm));
    const unsigned long memory_size (get_virtual_memory_per_node (vm));
    const unsigned long timeout (get_virtual_memory_timeout (vm));

    if (_rif_endpoints.empty())
    {
      throw std::runtime_error ("at least one node is required in machinefile");
    }

    const validators::executable vmem_binary
      ((installation.gspc_home() / "bin" / "gpi-space").string());

    if (boost::filesystem::exists (socket))
    {
      throw std::runtime_error ("socket file already exists: " + socket.string());
    }

    const std::string& master (_rif_endpoints.front().host);

    const std::string machinefile_path
      (rif_t::make_relative_to_rif_root ("nodefile").string());

    {
      std::vector<char> in_memory_hostlist;
      for (gspc::rif_t::endpoint_t const& rif: _rif_endpoints)
      {
        in_memory_hostlist.insert ( in_memory_hostlist.end()
                                  , rif.host.begin(), rif.host.end()
                                  );
        in_memory_hostlist.emplace_back ('\n');
      }
      _rif.store ( _rif_endpoints
                 , in_memory_hostlist
                 , machinefile_path
                 );
    }

    _rif.exec ( {_rif_endpoints.front()}
              , "gaspi-master"
              , { vmem_binary.string()
                , "--kvs-host", kvs_host, "--kvs-port", std::to_string (kvs_port)
                , "--log-host", log_host, "--log-port", std::to_string (log_port)
                , "--log-level", log_level
                , "--gpi-mem", std::to_string (memory_size)
                , "--socket", socket.string()
                , "--port", std::to_string (port)
                , "--gpi-api", _rif_endpoints.size() > 1 ? "gaspi" : "fake"
                , "--gpi-timeout", std::to_string (timeout)
                }
              , { {"GASPI_MFILE", machinefile_path}
                , {"GASPI_MASTER", master}
                , {"GASPI_SOCKET", "0"}
                , {"GASPI_TYPE", "GASPI_MASTER"}
                , {"GASPI_SET_NUMA_SOCKET", "0"}
                }
              );

    _rif.exec ( std::list<gspc::rif_t::endpoint_t> ( ++_rif_endpoints.begin()
                                                   , _rif_endpoints.end()
                                                   )
              , "gaspi-worker"
              , { vmem_binary.string()
                , "--kvs-host", kvs_host, "--kvs-port", std::to_string (kvs_port)
                , "--log-host", log_host, "--log-port", std::to_string (log_port)
                , "--log-level", log_level
                , "--gpi-mem", std::to_string (memory_size)
                , "--socket", socket.string()
                , "--port", std::to_string (port)
                , "--gpi-api", _rif_endpoints.size() > 1 ? "gaspi" : "fake"
                , "--gpi-timeout", std::to_string (timeout)
                }
              , { {"GASPI_MFILE", machinefile_path}
                , {"GASPI_MASTER", master}
                , {"GASPI_SOCKET", "0"}
                , {"GASPI_TYPE", "GASPI_WORKER"}
                , {"GASPI_SET_NUMA_SOCKET", "0"}
                }
              );

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
          + " waited: " + std::to_string (slept) + " ms"
          );
      }
    }
  }

  vmem_t::~vmem_t()
  {
    _rif.stop ( std::list<gspc::rif_t::endpoint_t> ( ++_rif_endpoints.begin()
                                                   , _rif_endpoints.end()
                                                   )
              , "gaspi-worker"
              );
    _rif.stop ({_rif_endpoints.front()}, "gaspi-master");
  }
}
