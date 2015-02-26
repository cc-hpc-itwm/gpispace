// bernd.loerwald@itwm.fraunhofer.de

#include <rif/strategy/poe.hpp>

#include <fhg/util/system_with_blocked_SIGCHLD.hpp>
#include <fhg/util/wait_and_collect_exceptions.hpp>

#include <boost/format.hpp>

#include <future>
#include <mutex>
#include <stdexcept>

namespace fhg
{
  namespace rif
  {
    namespace strategy
    {
      namespace poe
      {
        void bootstrap ( std::vector<std::string> const& hostnames
                       , boost::optional<unsigned short> const& port
                       , std::string const& register_host
                       , unsigned short register_port
                       , boost::filesystem::path const& binary
                       )
        {
          std::string const command
            ( ( boost::format
                ("poe %1% %2% --register-host %3% --register-port %4%")
                % binary
                % (port ? "--port " + std::to_string (*port) : "")
                % register_host
                % register_port
                ).str()
              );

          if ( int ec
             = fhg::util::system_with_blocked_SIGCHLD (command.c_str())
             )
          {
            throw std::runtime_error
              ( ( boost::format ("%1% failed: %2%")
                % command
                % ec
                ).str()
              );
          }
        }

        void teardown ( std::vector<fhg::rif::entry_point> const& entry_points
                      , std::vector<fhg::rif::entry_point>& failed_entry_points
                      )
        {
          if (entry_points.empty())
          {
            return;
          }

          std::ostringstream command;
          command << "(";
          for (fhg::rif::entry_point const& entry_point : entry_points)
          {
            command << "echo " << entry_point.to_string() << ";";
          }
          command << ") | "
                  << "poe \""
                    << "bash -c '"
                      << "cat | "
                      << "grep \\$(hostname) | "
                      << "cut -d\\\" \\\" -f 3 | "
                      << "xargs kill -TERM"
                    << "'"
                  << "\" "
                  << "-stdinmode all";

          if ( int ec
             = fhg::util::system_with_blocked_SIGCHLD (command.str().c_str())
             )
          {
            failed_entry_points = entry_points;

            throw std::runtime_error
              ( ( boost::format ("%1% failed: %2%")
                % command
                % ec
                ).str()
              );
          }
        }
      }
    }
  }
}
