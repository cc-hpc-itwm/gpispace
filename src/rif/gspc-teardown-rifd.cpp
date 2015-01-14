
#include <rif/entry_point.hpp>

#include <fhg/util/boost/asio/ip/address.hpp>
#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/join.hpp>
#include <fhg/util/nest_exceptions.hpp>
#include <fhg/util/print_exception.hpp>
#include <fhg/util/read_lines.hpp>
#include <fhg/util/system_with_blocked_SIGCHLD.hpp>
#include <fhg/util/wait_and_collect_exceptions.hpp>

#include <network/server.hpp>

#include <rpc/server.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/program_options.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <condition_variable>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace
{
  namespace option
  {
    constexpr const char* const entry_points_file {"entry-points-file"};
    constexpr const char* const strategy {"strategy"};
  }

  namespace strategy
  {
    void ssh ( std::vector<fhg::rif::entry_point> const& entry_points
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
                  if ( int ec
                     = fhg::util::system_with_blocked_SIGCHLD (command.c_str())
                     )
                  {
                    std::unique_lock<std::mutex> const _
                      (failed_entry_points_guard);
                    failed_entry_points.emplace_back (entry_point);

                    throw std::runtime_error
                      ( ( boost::format ("%1%: %2% failed: %3%")
                        % entry_point.hostname
                        % command
                        % ec
                        ).str()
                      );
                  }
                }
              )
          );
      }

      fhg::util::wait_and_collect_exceptions (sshs);
    }
  }
}

int main (int argc, char** argv)
try
{
  std::unordered_map
    < std::string
    , std::function<void ( std::vector<fhg::rif::entry_point> const&
                         , std::vector<fhg::rif::entry_point>&
                         )>
    > const strategies {{"ssh", strategy::ssh}};

  boost::program_options::options_description options_description;
  options_description.add_options()
    ( option::entry_points_file
    , boost::program_options::value
        <fhg::util::boost::program_options::existing_path>()->required()
    , "entry_points file"
    )
    ( option::strategy
    , boost::program_options::value<std::string>()->required()
    , ( "strategy: one of "
      + fhg::util::join (strategies | boost::adaptors::map_keys, ", ")
      ).c_str()
    )
    ;

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser (argc, argv)
      .options (options_description)
      .run()
    , vm
    );

  boost::program_options::notify (vm);

  std::string const strategy (vm.at (option::strategy).as<std::string>());

  if (!strategies.count (strategy))
  {
    throw std::invalid_argument
      (( boost::format ("invalid argument '%1%' for --%2%: one of %3%")
       % strategy
       % option::strategy
       % fhg::util::join (strategies | boost::adaptors::map_keys, ", ")
       ).str()
      );
  }

  std::vector<fhg::rif::entry_point> entry_points;
  for ( std::string const& line
      : fhg::util::read_lines
          ( vm.at (option::entry_points_file)
          . as<fhg::util::boost::program_options::existing_path>()
          )
      )
  {
    std::string hostname;
    unsigned short port;
    pid_t pid;
    std::istringstream iss (line);
    if (!(iss >> hostname >> port >> pid))
    {
      throw std::runtime_error
        ("parse error: expected 'host port pid': got '" + line + "'");
    }
    entry_points.emplace_back (hostname, port, pid);
  }

  fhg::util::nest_exceptions<std::runtime_error>
    ( [&]
      {
        std::vector<fhg::rif::entry_point> failed_entry_points;

        try
        {
          strategies.at (strategy) (entry_points, failed_entry_points);
        }
        catch (...)
        {
          for (fhg::rif::entry_point const& entry_point : failed_entry_points)
          {
            std::cout << entry_point.hostname
                      << ' ' << entry_point.port
                      << ' ' << entry_point.pid
                      << '\n';
          }

          throw;
        }
      }
    , "teardown-" + strategy + " failed: "
    );

  return 0;
}
catch (...)
{
  fhg::util::print_current_exception (std::cerr, "EX: ");

  return 1;
}
