// bernd.loerwald@itwm.fraunhofer.de

#include <drts/worker/start_drts_kernel.hpp>

#include <rif/execute_and_get_startup_messages.hpp>

#include <fhg/util/getenv.hpp>
#include <fhg/util/hostname.hpp>
#include <fhg/util/join.hpp>

#include <fstream>
#include <stdexcept>
#include <unordered_map>

#include <iostream>
namespace fhg
{
  namespace drts
  {
    namespace worker
    {
      namespace
      {
        void add_plugin_option ( std::vector<std::string>& arguments
                               , std::string name
                               , const char* value
                               )
        {
          arguments.emplace_back ("-s");
          arguments.emplace_back ("plugin." + name + "=" + value);
        }
        void add_plugin_option ( std::vector<std::string>& arguments
                               , std::string name
                               , std::string value
                               )
        {
          add_plugin_option (arguments, name, value.c_str());
        }
        void add_plugin_option ( std::vector<std::string>& arguments
                               , std::string name
                               , boost::filesystem::path value
                               )
        {
          add_plugin_option (arguments, name, value.string().c_str());
        }
        void add_plugin_option ( std::vector<std::string>& arguments
                               , std::string name
                               , unsigned long value
                               )
        {
          add_plugin_option (arguments, name, std::to_string (value).c_str());
        }

        std::vector<std::string> string_vector
          (std::vector<boost::filesystem::path> const& container)
        {
          std::vector<std::string> result;
          for (boost::filesystem::path const& elem : container)
          {
            result.emplace_back (elem.string());
          }
          return result;
        }
      }

      void start ( bool verbose
                 , std::string gui
                 , boost::optional<std::string> log_url
                 , std::string master
                 , unsigned long identity
                 , boost::optional<std::pair<boost::filesystem::path, unsigned long>> gpi
                 , std::vector<std::string> capabilities
                 , std::vector<boost::filesystem::path> lib_path
                 , boost::filesystem::path state_dir
                 , std::string name_prefix
                 , boost::optional<unsigned long> numa_socket
                 , boost::filesystem::path installation_dir
                 )
      {
        std::string const hostname (fhg::util::hostname());

        std::string const name
          ( name_prefix + "-" + hostname + "-" + std::to_string (identity)
          + (numa_socket ? ("." + std::to_string (numa_socket.get())) : std::string())
          );

        boost::filesystem::path const logprefix (state_dir / "log");

        boost::filesystem::create_directories
          (state_dir / "processes" / hostname);

        //! \todo do this?
        // exec 1> >(tee "${logprefix}/${name}.out")
        // exec 2> >(tee "${logprefix}/${name}.err" >&2)

        boost::filesystem::path const bin
          (installation_dir / "bin" / "drts-kernel");
        //! \todo also check for +x
        if (!boost::filesystem::exists (bin))
        {
          throw std::logic_error
            ("could not find drts-kernel at '" + bin.string() + "'");
        }

        std::vector<std::string> arguments;
        std::unordered_map<std::string, std::string> environment;

        arguments.emplace_back ("-n");
        arguments.emplace_back (name);
        arguments.emplace_back ("-L");
        arguments.emplace_back
          ((installation_dir / "libexec" / "fhg" / "plugins").string());

        add_plugin_option (arguments, "drts.master", master);
        add_plugin_option (arguments, "drts.backlog", 1);
        add_plugin_option (arguments, "drts.max_reconnect_attempts", 128);
        add_plugin_option
          (arguments, "drts.capabilities", fhg::util::join (capabilities, ","));
        add_plugin_option (arguments, "drts.gui_url", gui);
        add_plugin_option ( arguments
                          , "drts.library_path"
                          , fhg::util::join (string_vector (lib_path), ",")
                          );

        if (gpi)
        {
          capabilities.emplace_back ("GPI");

          arguments.emplace_back ("--gpi_enabled");
          add_plugin_option (arguments, "gpi.socket", gpi->first);
          add_plugin_option (arguments, "gpi.startmode", "wait");
          add_plugin_option (arguments, "gpi_compat.shm_size", gpi->second);
        }

        if (numa_socket)
        {
          add_plugin_option (arguments, "drts.socket", numa_socket.get());
        }

        environment.emplace ("FHGLOG_level", verbose ? "TRACE" : "INFO");
        environment.emplace
          ("FHGLOG_to_file", (logprefix / (name + ".log")).string());
        environment.emplace ( "LD_LIBRARY_PATH"
                            , (installation_dir / "lib").string() + ":"
                            + (installation_dir / "libexec" / "sdpa").string()
                            );

        if (log_url)
        {
          environment.emplace ("FHGLOG_to_server", log_url.get());
        }

        std::pair<pid_t, std::vector<std::string>> const pid_and_startup_messages
          ( fhg::rif::execute_and_get_startup_messages
              ("--startup-messages-pipe", "OKAY", bin, arguments, environment)
          );

        if (!pid_and_startup_messages.second.empty())
        {
          throw std::runtime_error
            ("could not start " + name + ": expected no startup messages");
        }

        std::ofstream pidfile
          ((state_dir / "processes" / hostname / ("drts-kernel-" + name + ".pid")).string());
        pidfile << pid_and_startup_messages.first;
      }
    }
  }
}
