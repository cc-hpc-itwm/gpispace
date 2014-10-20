// mirko.rahn@itwm.fraunhofer.de

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

namespace gspc
{
  namespace options
  {
    namespace name
    {
      constexpr char const* const log_host {"log-host"};
      constexpr char const* const log_port {"log-port"};
      constexpr char const* const log_level {"log-level"};
      constexpr char const* const gui_host {"gui-host"};
      constexpr char const* const gui_port {"gui-port"};

      constexpr char const* const state_directory {"state-directory"};
      constexpr char const* const gspc_home {"gspc-home"};
      constexpr char const* const nodefile {"nodefile"};
      constexpr char const* const application_search_path
        {"application-search-path"};

      constexpr char const* const virtual_memory_per_node
        {"virtual-memory-per-node"};
      constexpr char const* const virtual_memory_socket
        {"virtual-memory-socket"};
      constexpr char const* const virtual_memory_port
        {"virtual-memory-port"};
      constexpr char const* const virtual_memory_startup_timeout
        {"virtual-memory-startup-timeout"};
    }
  }

#define SET(_name, _type) \
  void set_ ## _name (boost::program_options::variables_map&, _type const&)
#define GET(_name, _type)                               \
  boost::optional<_type> get_ ## _name                  \
    (boost::program_options::variables_map const&)
#define ACCESS(_name, _type) SET (_name, _type); GET (_name, _type)

  ACCESS (log_host, std::string);
  ACCESS (log_port, unsigned short);
  ACCESS (log_level, std::string);
  ACCESS (gui_host, std::string);
  ACCESS (gui_port, unsigned short);

  ACCESS (state_directory, boost::filesystem::path);
  ACCESS (gspc_home, boost::filesystem::path);
  ACCESS (nodefile, boost::filesystem::path);
  GET (application_search_path, boost::filesystem::path);

  ACCESS (virtual_memory_per_node, unsigned long);
  GET (virtual_memory_socket, boost::filesystem::path);
  ACCESS (virtual_memory_port, unsigned short);
  ACCESS (virtual_memory_startup_timeout, unsigned long);

#undef ACCESS
#undef GET
#undef SET
}
