// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <drts/private/startup_and_shutdown.hpp>

#include <drts/private/drts_impl.hpp>
#include <drts/private/worker_description_implementation.hpp>

#include <rif/client.hpp>

#include <fhg/util/starts_with.hpp>
#include <util-generic/functor_visitor.hpp>
#include <util-generic/getenv.hpp>
#include <util-generic/hostname.hpp>
#include <util-generic/join.hpp>
#include <util-generic/make_optional.hpp>
#include <util-generic/nest_exceptions.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/read_file.hpp>
#include <util-generic/read_lines.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/serialization/boost/filesystem/path.hpp>
#include <util-generic/unreachable.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <rpc/remote_function.hpp>
#include <rpc/remote_tcp_endpoint.hpp>
#include <rpc/remote_socket_endpoint.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/utility/in_place_factory.hpp>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <functional>
#include <future>
#include <iterator>
#include <list>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <tuple>
#include <type_traits>

#include <unistd.h>

namespace fhg
{
  namespace drts
  {
    void processes_storage::store ( fhg::rif::entry_point const& entry_point
                                  , std::string const& name
                                  , pid_t pid
                                  )
    {
      std::lock_guard<std::mutex> const guard (_guard);

      if (!_[entry_point].emplace (name, pid).second)
      {
        throw std::logic_error
          ( "process with name '" + name + "' on entry point '"
          + entry_point.string() + "' already exists with pid "
          + std::to_string (_.at (entry_point).at (name))
          + ", new pid " + std::to_string (pid)
          );
      }
    }
    boost::optional<pid_t> processes_storage::pidof
      (fhg::rif::entry_point const& entry_point, std::string const& name)
    {
      std::lock_guard<std::mutex> const guard (_guard);

      auto pos_entry_point (_.find (entry_point));

      if (pos_entry_point == _.end())
      {
        return boost::none;
      }

      auto pos_name (pos_entry_point->second.find (name));

      if (pos_name == pos_entry_point->second.end())
      {
        return boost::none;
      }

      return pos_name->second;
    }
  }
}

namespace
{
  std::string build_parent_with_hostinfo
    (std::string const& name, fhg::drts::hostinfo_type const& hostinfo)
  {
    return ( boost::format ("%1%%%%2%%%%3%")
           % name
           % hostinfo.first
           % hostinfo.second
           ).str();
  }

  fhg::drts::hostinfo_type start_agent
    ( fhg::rif::entry_point const& rif_entry_point
    , fhg::rif::client& rif_client
    , std::string const& name
    , boost::optional<unsigned short> const& agent_port
    , boost::optional<boost::filesystem::path> const& gpi_socket
    , gspc::installation_path const& installation_path
    , fhg::drts::processes_storage& processes
    , std::ostream& info_output
    , boost::optional<std::pair<fhg::rif::client&, pid_t>> top_level_log
    , gspc::Certificates const& certificates
    )
  {
    info_output << "I: starting agent: " << name << " on rif entry point "
                << rif_entry_point;
    info_output << "\n";

    auto const result
      ( rif_client.start_agent
          ( name
          , agent_port
          , gpi_socket
          , certificates
          , installation_path.agent()
          ).get()
      );

    processes.store (rif_entry_point, name, result.pid);

    if (top_level_log)
    {
      top_level_log->first.add_emitter_to_logging_demultiplexer
        ( top_level_log->second
        , std::vector<fhg::logging::endpoint>
            {result.logger_registration_endpoint}
        ).get();
    }

    return result.hostinfo;
  }
}

namespace fhg
{
  namespace drts
  {
    std::pair< std::unordered_set<fhg::rif::entry_point>
             , std::unordered_map<fhg::rif::entry_point, std::exception_ptr>
             > start_workers_for
    ( std::vector<fhg::rif::entry_point> const& entry_points
    , std::string parent_name
    , fhg::drts::hostinfo_type parent_hostinfo
    , gspc::worker_description const& description_
    , fhg::drts::processes_storage& processes
    , boost::optional<boost::filesystem::path> const& gpi_socket
    , std::vector<boost::filesystem::path> const& app_path
    , std::vector<std::string> const& worker_env_copy_variable
    , bool worker_env_copy_current
    , std::vector<boost::filesystem::path> const& worker_env_copy_file
    , std::vector<std::string> const& worker_env_set_variable
    , gspc::installation_path const& installation_path
    , std::ostream& info_output
    , boost::optional<std::pair<fhg::rif::entry_point, pid_t>> top_level_log
    , gspc::Certificates const& certificates
    )
  {
     auto const& description (*description_._);

     std::string name_prefix (fhg::util::join (description.capabilities, '+').string());
     std::replace_if
       (name_prefix.begin(), name_prefix.end(), boost::is_any_of ("+#.-"), '_');

     info_output << "I: starting " << name_prefix << " workers (parent "
                 << parent_name << ", "
                 << description.num_per_node << "/host, "
                 << ( description.max_nodes == 0 ? "unlimited"
                    : description.max_nodes == 1 ? "unique"
                    : "global max: " + std::to_string (description.max_nodes)
                    )
                 << ", " << description.shm_size << " SHM) with parent "
                 << parent_name << " on rif entry point "
                 << fhg::util::join
                      ( entry_points
                      | boost::adaptors::transformed
                          ( [] (fhg::rif::entry_point const& entry_point)
                            {
                              return entry_point.string();
                            }
                          )
                      , ", "
                      )
                 << "\n";

     std::vector<std::string> arguments;

     arguments.emplace_back ("--parent");
     arguments.emplace_back
       (build_parent_with_hostinfo (parent_name, parent_hostinfo));

     for (boost::filesystem::path const& path : app_path)
     {
       arguments.emplace_back ("--library-search-path");
       arguments.emplace_back (path.string());
     }

     if (description.shm_size)
     {
       arguments.emplace_back ("--capability");
       arguments.emplace_back ("GPI");
       arguments.emplace_back ("--virtual-memory-socket");
       arguments.emplace_back (gpi_socket.get().string());
       arguments.emplace_back ("--shared-memory-size");
       arguments.emplace_back (std::to_string (description.shm_size));
     }

     for (std::string const& capability : description.capabilities)
     {
       arguments.emplace_back ("--capability");
       arguments.emplace_back (capability);
     }

     if (description.socket)
     {
       arguments.emplace_back ("--socket");
       arguments.emplace_back (std::to_string (description.socket.get()));
     }

     if (certificates)
     {
       arguments.emplace_back ("--certificates");
       arguments.emplace_back (certificates->string());
     }

     std::atomic<std::size_t> num_nodes (0);

      std::unordered_map<fhg::rif::entry_point, std::vector<std::exception_ptr>>
        exceptions;

      //! \todo let thread count be a parameter
      fhg::util::scoped_boost_asio_io_service_with_threads io_service
        (std::min (64UL, entry_points.size()));

      std::list<std::pair<rif::client, rif::entry_point>> rif_connections;
      util::nest_exceptions<std::runtime_error>
        ( [&]
          {
            for (rif::entry_point const& entry_point : entry_points)
            {
              rif_connections.emplace_back
                ( std::piecewise_construct
                , std::forward_as_tuple (io_service, entry_point)
                , std::forward_as_tuple (entry_point)
                );
            }
          }
        , "connecting to rif entry points"
        );

      std::vector<std::tuple< fhg::rif::entry_point
                            , std::future<fhg::rif::protocol::start_worker_result>
                            , std::string
                            >
                 > futures;

      for (auto& connection : rif_connections)
      {
        //! \todo does this work correctly for multi-segments?!
        if ( description.max_nodes != 0
           && num_nodes.fetch_add (1) >= description.max_nodes
           )
        {
          break;
        }

        for ( unsigned long identity (0)
            ; identity < description.num_per_node
            ; ++identity
            )
        {
          try
          {
            if (description.base_port)
            {
              arguments.emplace_back ("--port");
              arguments.emplace_back
                (std::to_string (*description.base_port + identity));
            }

            std::string const name
              ( name_prefix + "-" + connection.second.string()
              + "-" + std::to_string (identity + 1)
              + ( description.socket
                ? ("." + std::to_string (description.socket.get()))
                : std::string()
                )
              );
            std::string const storage_name ("drts-kernel-" + name);

            {
              boost::optional<pid_t> const mpid
                (processes.pidof (connection.second, storage_name));

              if (!!mpid)
              {
                throw std::logic_error
                  ( "process with name '" + name + "' on entry point '"
                  + connection.second.string() + "' already exists with pid "
                  + std::to_string (*mpid)
                  );
              }
            }

            std::unordered_map<std::string, std::string> environment;

            auto const& parse_and_add_definition
              ( [&] (std::string const& definition)
                {
                  auto const pos (definition.find ('='));
                  assert (pos != std::string::npos && pos != 0);
                  environment.emplace
                    (definition.substr (0, pos), definition.substr (pos + 1));
                }
              );

            for (auto const& key : worker_env_copy_variable)
            {
              auto const value (fhg::util::getenv (key.c_str()));
              if (!value)
              {
                throw std::invalid_argument
                  ( "requested to copy environment variable '" + key
                  + "', but variable is not set"
                  );
              }
              environment.emplace (key, *value);
            }

            if (worker_env_copy_current)
            {
              for (auto env (environ); *env; ++env)
              {
                parse_and_add_definition (*env);
              }
            }

            for (auto const& file : worker_env_copy_file)
            {
              for (auto const& definition : fhg::util::read_lines (file))
              {
                parse_and_add_definition (definition);
              }
            }

            for (auto const& definition : worker_env_set_variable)
            {
              parse_and_add_definition (definition);
            }

            futures.emplace_back
              ( connection.second
              , connection.first.start_worker
                  ( name
                  , installation_path.drts_kernel()
                  , arguments
                  , environment
                  )
              , name
              );
          }
          catch (...)
          {
            exceptions[connection.second].emplace_back
              (std::current_exception());
          }
        }
      }

      std::vector<fhg::logging::endpoint> log_emitters;

      for (auto& future : futures)
      {
        try
        {
          auto const result (std::get<1> (future).get());

          processes.store ( std::get<0> (future)
                          , "drts-kernel-" + std::get<2> (future)
                          , result.pid
                          );

          log_emitters.emplace_back
            (std::move (result.logger_registration_endpoint));
        }
        catch (...)
        {
          exceptions[std::get<0> (future)].emplace_back
            (std::current_exception());
        }
      }

      std::pair< std::unordered_set<fhg::rif::entry_point>
               , std::unordered_map<fhg::rif::entry_point, std::exception_ptr>
               > results;

      for (auto const& connection : rif_connections)
      {
        try
        {
          auto const it (exceptions.find ((connection.second)));
          if (it == exceptions.end())
          {
            results.first.emplace (connection.second);
          }
          else
          {
            //! \todo return the individual exceptions
            fhg::util::throw_collected_exceptions (it->second);
          }
        }
        catch (...)
        {
          results.second.emplace (connection.second, std::current_exception());
        }
      }

      if (top_level_log)
      {
        fhg::rif::client (io_service, top_level_log->first)
          .add_emitter_to_logging_demultiplexer
            (top_level_log->second, log_emitters).get();
      }

      return results;
    }

    namespace
    {
      std::unique_ptr<rpc::remote_endpoint> connect
        (boost::asio::io_service& io_service, logging::endpoint ep)
      {
        return util::visit<std::unique_ptr<rpc::remote_endpoint>>
          ( ep.best (util::hostname())
          , [&] (logging::socket_endpoint const& as_socket)
            {
              return std::make_unique<rpc::remote_socket_endpoint>
                (io_service, as_socket.socket);
            }
          , [&] (logging::tcp_endpoint const& as_tcp)
            {
              return std::make_unique<rpc::remote_tcp_endpoint>
                (io_service, as_tcp);
            }
          );
      }
    }

    startup_result startup
      ( boost::optional<unsigned short> const& agent_port
      , boost::optional<boost::filesystem::path> gpi_socket
      , gspc::installation_path const& installation_path
      , fhg::util::signal_handler_manager& signal_handler_manager
      , std::vector<fhg::rif::entry_point> const& rif_entry_points
      , fhg::rif::entry_point const& parent
      , fhg::drts::processes_storage& processes
      , std::string& parent_agent_name
      , fhg::drts::hostinfo_type& parent_agent_hostinfo
      , std::ostream& info_output
      , boost::optional<fhg::rif::entry_point> log_rif_entry_point
      , std::vector<logging::endpoint> default_log_receivers
      , gspc::Certificates const& certificates
      )
    {
      fhg::util::scoped_signal_handler interrupt_signal_handler
        ( signal_handler_manager
        ,  SIGINT
        , [] (int, siginfo_t*, void*)
          {
            throw std::runtime_error ("Canceled...");
          }
        );

      info_output << "I: starting base sdpa components on " << parent << "...\n";

      //! \todo let thread count be a parameter
      fhg::util::scoped_boost_asio_io_service_with_threads io_service
        (std::max (1UL, std::min (64UL, rif_entry_points.size())));

      rif::client parent_rif_client (io_service, parent);

      boost::optional<rif::client> logging_rif_client;
      boost::optional<rif::protocol::start_logging_demultiplexer_result>
        logging_rif_info;
      if (log_rif_entry_point)
      {
        logging_rif_client = boost::in_place
          (std::ref (io_service), *log_rif_entry_point);

        info_output << "I: starting top level gspc logging demultiplexer on "
                    << log_rif_entry_point->hostname << "\n";

        logging_rif_info
          = util::nest_exceptions<std::runtime_error>
              ( [&]
                {
                  return logging_rif_client->start_logging_demultiplexer
                    (installation_path.logging_demultiplexer()).get();
                }
              , "Starting top level logging demultiplexer failed"
              );

        info_output << "   => accepting registration on '"
                    << logging_rif_info->sink_endpoint.to_string()
                    << "'\n";

        processes.store
          (*log_rif_entry_point, "logging-demultiplexer", logging_rif_info->pid);

        std::vector<logging::endpoint> const top_level_endpoint
          {logging_rif_info->sink_endpoint};
        for (auto const& receiver : default_log_receivers)
        {
          util::nest_exceptions<std::runtime_error>
            ( [&]
              {
                auto const endpoint (connect (io_service, receiver));
                using fun = logging::protocol::receiver::add_emitters;
                rpc::sync_remote_function<fun> {*endpoint} (top_level_endpoint);
              }
            , "Requesting " + receiver.to_string()
            + " to connect to top level logging demultiplexer failed"
            );
        }
      }

      std::list<std::pair<rif::client, rif::entry_point>> rif_connections;
      std::vector<std::string> hostnames;
      util::nest_exceptions<std::runtime_error>
        ( [&]
          {
            for (rif::entry_point const& entry_point : rif_entry_points)
            {
              rif_connections.emplace_back
                ( std::piecewise_construct
                , std::forward_as_tuple (io_service, entry_point)
                , std::forward_as_tuple (entry_point)
                );
              hostnames.emplace_back (entry_point.hostname);
            }
          }
        , "connecting to rif entry points"
        );

      if (gpi_socket)
      {
        info_output << "I: external IML as VMEM running on: "
                    << gpi_socket.get()
                    << "\n";
      }

      parent_agent_name = "agent-" + parent.string() + "-0";

      parent_agent_hostinfo = start_agent ( parent
                                          , parent_rif_client
                                          , parent_agent_name
                                          , agent_port
                                          , gpi_socket
                                          , installation_path
                                          , processes
                                          , info_output
                                          , FHG_UTIL_MAKE_OPTIONAL
                                              ( !!logging_rif_client
                                              , std::make_pair
                                                  ( std::ref (*logging_rif_client)
                                                  , logging_rif_info->pid
                                                  )
                                              )
                                          , certificates
                                          );

      return {parent_agent_hostinfo, logging_rif_info};
    }

    namespace
    {
      template<typename It>
        std::unordered_map
          < rif::entry_point
          , std::pair< std::string /* kind */
                     , std::unordered_map<pid_t, std::exception_ptr>
                     >
          > terminate_all_processes_of_a_kind
              ( std::vector<It> const& entry_point_procs
              , component_type component
              , std::ostream& info_output
              )
      {
        std::string const kind = [](component_type c)
        {
          switch (c)
          {
            case component_type::worker:
              return "drts-kernel";
            case component_type::agent:
              return "agent";
            case component_type::logging_demultiplexer:
              return "logging-demultiplexer";
          }

          FHG_UTIL_UNREACHABLE();
        }(component);

        std::unordered_map
          < rif::entry_point
          , std::pair< std::string /* kind */
                     , std::unordered_map<pid_t, std::exception_ptr>
                     >
          > failures;

        std::size_t processes_to_kill (0);
        for (auto const& entry_point_processes : entry_point_procs)
        {
          for (auto const& it : entry_point_processes->second)
          {
            if (fhg::util::starts_with (kind, it.first))
            {
              ++processes_to_kill;
            }
          }
        }

        //! \todo let thread count be a parameter
        fhg::util::scoped_boost_asio_io_service_with_threads io_service
          (std::min (64UL, processes_to_kill));

        using process_iter
          = typename decltype (entry_point_procs.front()->second)::iterator;

        std::list<fhg::rif::client> clients;
        std::vector<std::tuple < It
                               , std::future<std::unordered_map<pid_t, std::exception_ptr>>
                               , std::unordered_map<pid_t, process_iter>
                               , std::function<void()>
                               >
                   > futures;

        for (auto const& entry_point_processes : entry_point_procs)
        {
          std::vector<pid_t> pids;
          std::unordered_map<pid_t, process_iter> to_erase;
          for ( process_iter it (entry_point_processes->second.begin())
              ; it != entry_point_processes->second.end()
              ; ++it
              )
          {
            if (fhg::util::starts_with (kind, it->first))
            {
              to_erase.emplace (it->second, it);
              pids.emplace_back (it->second);
            }
          }

          auto const& entry_point (entry_point_processes->first);
          auto&& fail_group_with_current_exception
            ( [&failures, pids, entry_point, &kind]
              {
                std::unordered_map<pid_t, std::exception_ptr> fails;

                for (pid_t pid : pids)
                {
                  fails.emplace (pid, std::current_exception());
                }

                failures.emplace ( entry_point
                                 , std::make_pair (kind, fails)
                                 );
              }
            );

          if (!pids.empty())
          {
            info_output << "terminating " << kind << " on "
                        << entry_point
                        << ": " << fhg::util::join (pids, ' ') << "\n";

            try
            {
              clients.emplace_back (io_service, entry_point);
              futures.emplace_back ( entry_point_processes
                                   , clients.back().kill (pids)
                                   , std::move (to_erase)
                                   , fail_group_with_current_exception
                                   );
            }
            catch (...) // \note: e.g. rif::client::connect
            {
              fail_group_with_current_exception();
            }
          }
        }

        for (auto& future : futures)
        {
          try
          {
            auto const failures_kill (std::get<1> (future).get());

            if (!failures_kill.empty())
            {
              failures.emplace
                ( std::get<0> (future)->first
                , std::make_pair (kind, failures_kill)
                );
            }
          }
          catch (...)
          {
            std::get<3> (future)();
          }

          //! \note: remove the process from the list of known
          //! processes in case of failure too assumption: when kill
          //! failed once, it will never succeed
          for (auto const& iter : std::get<2> (future))
          {
            std::get<0> (future)->second.erase (iter.second);
          }
        }

        return failures;
      }
    }

    std::unordered_map< rif::entry_point
                      , std::pair< std::string /* kind */
                                 , std::unordered_map<pid_t, std::exception_ptr>
                                 >
                      >
      processes_storage::shutdown_worker
        (std::vector<fhg::rif::entry_point> const& rif_entry_points)
    {
      std::vector<decltype (_)::iterator> iterators;
      for (fhg::rif::entry_point const& entry_point : rif_entry_points)
      {
        decltype (_)::iterator const pos (_.find (entry_point));
        if (pos == _.end())
        {
          throw std::invalid_argument ("shutdown for unknown entry_point");
        }
        //! \todo detect if there are any components on this entry_point
        iterators.emplace_back (pos);
      }

      std::unordered_map
        < rif::entry_point
        , std::pair< std::string /* kind */
                   , std::unordered_map<pid_t, std::exception_ptr>
                   >
        > const failures
            (terminate_all_processes_of_a_kind ( iterators
                                               , component_type::worker
                                               , _info_output
                                               )
            );

      for (decltype (_)::iterator const& it : iterators)
      {
        if (it->second.empty())
        {
          _.erase (it);
        }
      }

      return failures;
    }

    namespace
    {
      template<typename It>
        std::vector<It> iterators (It begin, It end)
      {
        std::vector<It> res (std::distance (begin, end));
        std::iota (res.begin(), res.end(), begin);
        return res;
      }
    }

    processes_storage::~processes_storage()
    {
      util::apply_for_each_and_collect_exceptions
        ( { component_type::worker
          , component_type::agent
          , component_type::logging_demultiplexer
          }
        , [this] (component_type component)
          {
            std::unordered_map
              < rif::entry_point
              , std::pair< std::string /* kind */
                         , std::unordered_map<pid_t, std::exception_ptr>
                         >
              > const failures
                  ( terminate_all_processes_of_a_kind
                      (iterators (_.begin(), _.end()), component, _info_output)
                  );

            for (auto const& failure : failures)
            {
              for (auto const& fails : failure.second.second)
              {
                _info_output <<
                  ( boost::format ("Could not terminate %1%[%2%] on %3%: %4%")
                  % failure.second.first
                  % fails.first
                  % failure.first
                  % util::exception_printer (fails.second)
                  ) << std::endl;
              }
            }
          }
        );
    }
  }
}
