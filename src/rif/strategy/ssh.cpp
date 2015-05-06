// bernd.loerwald@itwm.fraunhofer.de

#include <rif/strategy/ssh.hpp>

#include <util-generic/nest_exceptions.hpp>
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
      namespace ssh
      {
        void bootstrap ( std::vector<std::string> const& hostnames
                       , boost::optional<unsigned short> const& port
                       , std::string const& register_host
                       , unsigned short register_port
                       , boost::filesystem::path const& binary
                       )
        {
          std::vector<std::future<void>> sshs;

          for (std::string const& hostname : hostnames)
          {
            std::string const command
              ( ( boost::format
                    ("ssh %1% %2% %3% %4% --register-host %5% --register-port %6%")
                % "-q -x -T -n -o CheckHostIP=no -o StrictHostKeyChecking=no"
                % hostname
                % binary
                % (port ? "--port " + std::to_string (*port) : "")
                % register_host
                % register_port
                ).str()
              );

            sshs.emplace_back
              ( std::async
                  ( std::launch::async
                  , [hostname, command]
                    {
                      fhg::util::nest_exceptions<std::runtime_error>
                        ([&hostname, &command]()
                         {
                           fhg::util::system_with_blocked_SIGCHLD (command);
                         }
                        , hostname
                        );
                    }
                  )
              );
          }

          fhg::util::wait_and_collect_exceptions (sshs);
        }

        void teardown ( std::vector<fhg::rif::entry_point> const& entry_points
                      , std::vector<fhg::rif::entry_point>& failed_entry_points
                      )
        {
          std::mutex failed_entry_points_guard;

          std::vector<std::future<void>> sshs;

          for (fhg::rif::entry_point const& entry_point : entry_points)
          {
            std::string const command
              ( ( boost::format ("ssh %1% %2% /bin/kill -TERM %3%")
                % "-q -x -T -n -o CheckHostIP=no -o StrictHostKeyChecking=no"
                % entry_point.hostname
                % entry_point.pid
                ).str()
              );

            sshs.emplace_back
              ( std::async
                  ( std::launch::async
                  , [ command, entry_point
                    , &failed_entry_points_guard, &failed_entry_points
                    ]
                    {
                      fhg::util::nest_exceptions<std::runtime_error>
                        ([&]()
                         {
                           try
                           {
                             fhg::util::system_with_blocked_SIGCHLD (command);
                           }
                           catch (...)
                           {
                             std::unique_lock<std::mutex> const _
                               (failed_entry_points_guard);
                             failed_entry_points.emplace_back (entry_point);

                             throw;
                           }
                         }
                        , entry_point.hostname
                        );
                    }
                  )
              );
          }

          fhg::util::wait_and_collect_exceptions (sshs);
        }
      }
    }
  }
}
