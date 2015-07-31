// bernd.loerwald@itwm.fraunhofer.de

#include <rif/strategy/ssh.hpp>

#include <util-generic/nest_exceptions.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>
#include <fhg/util/system_with_blocked_SIGCHLD.hpp>

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
        std::unordered_map<std::string, std::exception_ptr>
          bootstrap ( std::vector<std::string> const& hostnames
                    , boost::optional<unsigned short> const& port
                    , std::string const& register_host
                    , unsigned short register_port
                    , boost::filesystem::path const& binary
                    )
        {
          std::mutex failed_guard;
          std::unordered_map<std::string, std::exception_ptr> failed;

          std::vector<std::future<void>> sshs;

          for (std::string const& hostname : hostnames)
          {
            std::string const command
              ( ( boost::format
                    ("ssh %1% %2% %3% %4% --register-host %5% --register-port %6%"
                    " --register-key %7%"
                    )
                % "-q -x -T -n -o CheckHostIP=no -o StrictHostKeyChecking=no"
                % hostname
                % binary
                % (port ? "--port " + std::to_string (*port) : "")
                % register_host
                % register_port
                % hostname
                ).str()
              );

            sshs.emplace_back
              ( std::async
                  ( std::launch::async
                  , [hostname, command, &failed, &failed_guard]
                    {
                      try
                      {
                        fhg::util::system_with_blocked_SIGCHLD (command);
                      }
                      catch (...)
                      {
                        std::unique_lock<std::mutex> const _ (failed_guard);
                        failed.emplace (hostname, std::current_exception());
                      }
                    }
                  )
              );
          }

          for (auto& ssh : sshs)
          {
            ssh.get();
          }

          return failed;
        }

        std::pair < std::unordered_set<std::string>
                  , std::unordered_map<std::string, std::exception_ptr>
                  > teardown
          (std::unordered_map<std::string, fhg::rif::entry_point> const&
            entry_points
          )
        {
          std::mutex guard;
          std::unordered_set<std::string> terminated;
          std::unordered_map<std::string, std::exception_ptr> failed;

          std::vector<std::future<void>> sshs;

          for ( std::pair<std::string, fhg::rif::entry_point> const& entry_point
              : entry_points
              )
          {
            std::string const command
              ( ( boost::format ("ssh %1% %2% /bin/kill -TERM %3%")
                % "-q -x -T -n -o CheckHostIP=no -o StrictHostKeyChecking=no"
                % entry_point.second.hostname
                % entry_point.second.pid
                ).str()
              );

            sshs.emplace_back
              ( std::async
                  ( std::launch::async
                  , [ command, entry_point
                    , &guard, &failed, &terminated
                    ]
                    {
                      try
                      {
                        fhg::util::system_with_blocked_SIGCHLD (command);

                        std::unique_lock<std::mutex> const _ (guard);
                        terminated.emplace (entry_point.first);
                      }
                      catch (...)
                      {
                        std::unique_lock<std::mutex> const _ (guard);
                        failed.emplace
                          (entry_point.first, std::current_exception());
                      }
                    }
                  )
              );
          }

          for (auto& ssh : sshs)
          {
            ssh.get();
          }

          return {terminated, failed};
        }
      }
    }
  }
}
