#include <drts/private/option.hpp>
#include <drts/private/vmem.hpp>
#include <drts/private/rif.hpp>

#include <fhg/util/boost/program_options/validators/executable.hpp>
#include <fhg/util/make_unique.hpp>

#include <chrono>
#include <future>
#include <thread>
#include <unordered_set>

namespace gspc
{
  vmem_t::vmem_t
    ( boost::program_options::variables_map const& vm
    , gspc::installation const& installation
    , gspc::rif_t& rif
    , boost::filesystem::path const& nodefile_path
    , std::pair<std::list<std::string>, unsigned long> const& machinefile
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

    const boost::filesystem::path socket (require_virtual_memory_socket (vm));

    if (boost::filesystem::exists (socket))
    {
      throw std::runtime_error
        ("socket file already exists: " + socket.string());
    }

    const std::string& master (_rif_endpoints.front().host);

    const std::string log_host (require_log_host (vm));
    const unsigned short log_port (require_log_port (vm));
    const std::string log_level (require_log_level (vm));
    const unsigned short port (require_virtual_memory_port (vm));
    const unsigned long memory_size (require_virtual_memory_per_node (vm));
    const unsigned long startup_timeout_in_seconds
      (require_virtual_memory_startup_timeout (vm));

    std::future<void> master_startup
      ( std::async
          ( std::launch::async
          , [&]
            {
              _rif.exec
                ( {_rif_endpoints.front()}
                , "gaspi-master"
                , "--startup-messages-pipe"
                , "OKAY"
                , vmem_binary
                , { "--log-host", log_host, "--log-port", std::to_string (log_port)
                  , "--log-level", log_level
                  , "--gpi-mem", std::to_string (memory_size)
                  , "--socket", socket.string()
                  , "--port", std::to_string (port)
                  , "--gpi-api", _rif_endpoints.size() > 1 ? "gaspi" : "fake"
                  , "--gpi-timeout", std::to_string (startup_timeout_in_seconds)
                  }
                , { {"GASPI_MFILE", nodefile_path.string()}
                  , {"GASPI_MASTER", master}
                  , {"GASPI_SOCKET", "0"}
                  , {"GASPI_TYPE", "GASPI_MASTER"}
                  , {"GASPI_SET_NUMA_SOCKET", "0"}
                  }
                , installation.gspc_home()
                );
            }
          )
      );

    _rif.exec
      ( std::list<gspc::rif_t::endpoint_t> ( ++_rif_endpoints.begin()
                                           , _rif_endpoints.end()
                                           )
      , "gaspi-worker"
      , "--startup-messages-pipe"
      , "OKAY"
      , vmem_binary
      , { "--log-host", log_host, "--log-port", std::to_string (log_port)
        , "--log-level", log_level
        , "--gpi-mem", std::to_string (memory_size)
        , "--socket", socket.string()
        , "--port", std::to_string (port)
        , "--gpi-api", _rif_endpoints.size() > 1 ? "gaspi" : "fake"
        , "--gpi-timeout", std::to_string (startup_timeout_in_seconds)
        }
      , { {"GASPI_MFILE", nodefile_path.string()}
        , {"GASPI_MASTER", master}
        , {"GASPI_SOCKET", "0"}
        , {"GASPI_TYPE", "GASPI_WORKER"}
        , {"GASPI_SET_NUMA_SOCKET", "0"}
        }
      , installation.gspc_home()
      );

    master_startup.get();
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
