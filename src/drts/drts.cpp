// mirko.rahn@itwm.fraunhofer.de

#include <drts/drts.hpp>
#include <drts/private/option.hpp>

#include <drts/virtual_memory.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/type/activity.hpp>
//! \todo eliminate this include (that completes type transition_t::data)
#include <we/type/net.hpp>

#include <gpi-space/pc/client/api.hpp>

#include <sdpa/client.hpp>

#include <fhg/util/make_unique.hpp>
#include <fhg/util/read_file.hpp>
#include <fhg/util/system_with_blocked_SIGCHLD.hpp>

#include <boost/format.hpp>

#include <chrono>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <unordered_set>

namespace gspc
{
  namespace
  {
    std::pair<std::list<std::string>, unsigned long>
      read_nodes (boost::filesystem::path const& nodefile)
    {
      std::unordered_set<std::string> unique_nodes;
      std::list<std::string> nodes;

      {
        std::ifstream stream (nodefile.string());

        std::string node;

        while (std::getline (stream, node))
        {
          unique_nodes.insert (node);
          nodes.emplace_back (node);
        }
      }

      if (unique_nodes.empty())
      {
        throw std::runtime_error
          (( boost::format ("nodefile %1% does not contain nodes")
           % nodefile
           ).str()
          );
      }

      return std::make_pair (nodes, unique_nodes.size());
    }

    void system (std::string const& command, std::string const& description)
    {
      if (int ec = fhg::util::system_with_blocked_SIGCHLD (command.c_str()))
      {
        throw std::runtime_error
          (( boost::format
             ("Could not '%3%': error code '%1%', command was '%2%'")
           % ec
           % command
           % description
           ).str()
          );
      };
    }
  }

  installation::installation
    (boost::program_options::variables_map const& vm)
      : _gspc_home (boost::filesystem::canonical (require_gspc_home (vm)))
  {}

  scoped_runtime_system::scoped_runtime_system
    ( boost::program_options::variables_map const& vm
    , installation const& installation
    , std::string const& topology_description
    )
      : _installation (installation)
      , _state_directory (require_state_directory (vm))
      , _nodefile (boost::filesystem::canonical (require_nodefile (vm)))
      , _virtual_memory_per_node (get_virtual_memory_per_node (vm))
      , _virtual_memory_socket (get_virtual_memory_socket (vm))
      , _virtual_memory_startup_timeout
        ( get_virtual_memory_startup_timeout (vm)
        ? boost::make_optional
          (std::chrono::seconds (get_virtual_memory_startup_timeout (vm).get()))
        : boost::none
        )
      , _nodes_and_number_of_unique_nodes (read_nodes (_nodefile))
      , _virtual_memory_api
        ( _virtual_memory_socket
        ? fhg::util::make_unique<gpi::pc::client::api_t>
          (_virtual_memory_socket->string())
        : nullptr
        )
  {
    std::ostringstream command_boot;

    command_boot
      << (_installation.gspc_home() / "bin" / "sdpa")
      << " -s " << _state_directory
      << " boot"
      << " -f " << _nodefile;

    if (get_log_host (vm))
    {
      command_boot << " -l " << get_log_host (vm).get();

      if (get_log_port (vm))
      {
        command_boot << ":" << get_log_port (vm).get();
      }
    }

    if (get_gui_host (vm))
    {
      command_boot << " -g " << get_gui_host (vm).get();

      if (get_gui_port (vm))
      {
        command_boot << ":" << get_gui_port (vm).get();
      }
    }

    if (_virtual_memory_per_node)
    {
      command_boot
        << " -y " << *_virtual_memory_socket
        << " -m " << *_virtual_memory_per_node
        << " -T " << _virtual_memory_startup_timeout->count()
        << " -P " << require_virtual_memory_port (vm)
        ;
    }
    else
    {
      command_boot << " -M";
    }

    if (get_application_search_path (vm))
    {
      for ( boost::filesystem::path const& path
          : {get_application_search_path (vm).get()}
          )
      {
        command_boot << " -A " << boost::filesystem::canonical (path);
      }
    }

    command_boot << " " << topology_description;

    system (command_boot.str(), "start runtime system");

    // taken from pbs/sdpa and bin/sdpac
    //! \todo Remove magic: specify filenames instead of relying on
    //! file? Let an c++-ified sdpa-boot() return them.
    _orchestrator_host = fhg::util::read_file
      (_state_directory / "orchestrator.host");
    _orchestrator_port = boost::lexical_cast<unsigned short>
      (fhg::util::read_file (_state_directory / "orchestrator.port"));

    if (_virtual_memory_per_node)
    {
      _virtual_memory_api->start();
    }
  }

  scoped_runtime_system::~scoped_runtime_system()
  {
    _virtual_memory_api.reset();

    system ( ( boost::format ("%1% -s %2% stop")
             % (_installation.gspc_home() / "bin" / "sdpa")
             % _state_directory
             ).str()
           , "stop runtime system"
           );
  }

  vmem_allocation scoped_runtime_system::alloc
    (unsigned long size, std::string const& description) const
  {
    return vmem_allocation (this, size, description);
  }
}
