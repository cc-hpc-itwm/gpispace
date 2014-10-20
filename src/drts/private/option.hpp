// mirko.rahn@itwm.fraunhofer.de

#include <boost/filesystem.hpp>
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

  void set_gspc_home ( boost::program_options::variables_map&
                     , boost::filesystem::path const&
                     );
  void set_state_directory ( boost::program_options::variables_map&
                           , boost::filesystem::path const&
                           );
  void set_nodefile ( boost::program_options::variables_map&
                    , boost::filesystem::path const&
                    );
  void set_virtual_memory_per_node ( boost::program_options::variables_map&
                                   , unsigned long
                                   );
  unsigned long get_virtual_memory_per_node (boost::program_options::variables_map const&);
  unsigned short get_virtual_memory_port (boost::program_options::variables_map const&);
  unsigned long get_virtual_memory_startup_timeout (boost::program_options::variables_map const&);
  boost::filesystem::path
  get_not_yet_existing_virtual_memory_socket (boost::program_options::variables_map const&);

  void set_log_host ( boost::program_options::variables_map&
                    , std::string const&
                    );
  std::string get_log_host (boost::program_options::variables_map const&);
  void set_log_level ( boost::program_options::variables_map&
                     , std::string const&
                     );
  std::string get_log_level (boost::program_options::variables_map const&);
  void set_gui_host ( boost::program_options::variables_map&
                    , std::string const&
                    );
  std::string get_gui_host (boost::program_options::variables_map const&);
  void set_log_port ( boost::program_options::variables_map&
                    , unsigned short
                    );
  unsigned short get_log_port (boost::program_options::variables_map const&);
  void set_gui_port ( boost::program_options::variables_map&
                    , unsigned short
                    );
  unsigned short get_gui_port (boost::program_options::variables_map const&);
}
