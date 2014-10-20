#include <drts/private/option.hpp>
#include <drts/private/vmem.hpp>
#include <drts/private/rif.hpp>

#include <fhg/util/boost/program_options/validators/executable.hpp>
#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/boost/program_options/validators/is_directory_if_exists.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <fhg/util/boost/program_options/validators/nonexisting_path.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/make_unique.hpp>

#include <chrono>
#include <thread>
#include <unordered_set>

namespace gspc
{
  vmem_t::vmem_t
    ( boost::program_options::variables_map const& vm
    , gspc::installation const& installation
    , gspc::rif_t& rif
    , std::pair<std::list<std::string>, unsigned long> const& machinefile
    , const std::string& kvs_host
    , const unsigned short kvs_port
    )
    : _rif (rif)
    , _rif_endpoints
      ( [] (const std::list<std::string>& hosts)
        -> std::list<gspc::rif_t::endpoint_t>
        {
          std::list<gspc::rif_t::endpoint_t> ep;
          std::unordered_set<std::string> seen;
          for (std::string const& h : hosts)
          {
            if (seen.emplace (h).second)
            {
              ep.emplace_back (h, 22);
            }
          }
          return ep;
        } (machinefile.first)
      )
  {
    if (_rif_endpoints.empty())
    {
      throw std::runtime_error ("at least one node is required in machinefile");
    }

    const fhg::util::boost::program_options::executable vmem_binary
      ((installation.gspc_home() / "bin" / "gpi-space").string());

    const boost::filesystem::path socket
      (get_not_yet_existing_virtual_memory_socket (vm));

    if (boost::filesystem::exists (socket))
    {
      throw std::runtime_error
        ("socket file already exists: " + socket.string());
    }

    const std::string& master (_rif_endpoints.front().host);

    const std::string nodefile_path
      (rif_t::make_relative_to_rif_root ("nodefile").string());

    {
      std::string in_memory_hostlist;
      for (gspc::rif_t::endpoint_t const& rif: _rif_endpoints)
      {
        in_memory_hostlist += rif.host;
        in_memory_hostlist += "\n";
      }
      _rif.store ( _rif_endpoints
                 , in_memory_hostlist
                 , nodefile_path
                 );
    }

    const std::string log_host (get_log_host (vm));
    const unsigned short log_port (get_log_port (vm));
    const std::string log_level (get_log_level (vm));
    const unsigned short port (get_virtual_memory_port (vm));
    const unsigned long memory_size (get_virtual_memory_per_node (vm));
    const unsigned long startup_timeout_in_seconds
      (get_virtual_memory_startup_timeout (vm));

    _rif.exec
      ( {_rif_endpoints.front()}
      , "gaspi-master"
      , { vmem_binary.string()
        , "--kvs-host", kvs_host, "--kvs-port", std::to_string (kvs_port)
        , "--log-host", log_host, "--log-port", std::to_string (log_port)
        , "--log-level", log_level
        , "--gpi-mem", std::to_string (memory_size)
        , "--socket", socket.string()
        , "--port", std::to_string (port)
        , "--gpi-api", _rif_endpoints.size() > 1 ? "gaspi" : "fake"
        , "--gpi-timeout", std::to_string (startup_timeout_in_seconds)
        }
      , { {"GASPI_MFILE", nodefile_path}
        , {"GASPI_MASTER", master}
        , {"GASPI_SOCKET", "0"}
        , {"GASPI_TYPE", "GASPI_MASTER"}
        , {"GASPI_SET_NUMA_SOCKET", "0"}
        }
      );

    _rif.exec
      ( std::list<gspc::rif_t::endpoint_t> ( ++_rif_endpoints.begin()
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
        , "--gpi-timeout", std::to_string (startup_timeout_in_seconds)
        }
      , { {"GASPI_MFILE", nodefile_path}
        , {"GASPI_MASTER", master}
        , {"GASPI_SOCKET", "0"}
        , {"GASPI_TYPE", "GASPI_WORKER"}
        , {"GASPI_SET_NUMA_SOCKET", "0"}
        }
      );

    std::chrono::steady_clock::time_point const until
      ( std::chrono::steady_clock::now()
      + std::chrono::seconds (startup_timeout_in_seconds)
      );

    while (std::chrono::steady_clock::now() < until)
    {
      if (boost::filesystem::exists (socket))
      {
        break;
      }

      std::this_thread::sleep_for
        (std::min ( std::chrono::milliseconds (200)
                  , std::chrono::duration_cast<std::chrono::milliseconds>
                    (until - std::chrono::steady_clock::now())
                  )
        );
    }

    if (!boost::filesystem::exists (socket))
    {
      throw std::runtime_error
        ("timeout while waiting for: " + socket.string());
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
