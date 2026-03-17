// Copyright (C) 2014-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/drts/private/startup_and_shutdown.hpp>

#include <gspc/drts/private/drts_impl.hpp>
#include <gspc/drts/private/worker_description_implementation.hpp>

#include <gspc/rif/client.hpp>

#include <gspc/util/starts_with.hpp>
#include <gspc/util/functor_visitor.hpp>
#include <gspc/util/hostname.hpp>
#include <gspc/util/join.hpp>
#include <gspc/util/make_optional.hpp>
#include <gspc/util/print_exception.hpp>
#include <gspc/util/read_file.hpp>
#include <gspc/util/read_lines.hpp>
#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>
#include <gspc/util/unreachable.hpp>
#include <gspc/util/wait_and_collect_exceptions.hpp>

#include <gspc/rpc/remote_function.hpp>
#include <gspc/rpc/remote_socket_endpoint.hpp>
#include <gspc/rpc/remote_tcp_endpoint.hpp>

#include <boost/asio/io_service.hpp>

#include <boost/serialization/unordered_map.hpp>

#include <filesystem>
#include <optional>
#include <boost/utility/in_place_factory.hpp>

#include <gspc/util/exception_printer.formatter.hpp>
#include <gspc/rif/entry_point.formatter.hpp>
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdlib>
#include <exception>
#include <fmt/core.h>
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


  namespace fhg::drts
  {
    void processes_storage::store ( gspc::rif::entry_point const& entry_point
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
    std::optional<pid_t> processes_storage::pidof
      (gspc::rif::entry_point const& entry_point, std::string const& name)
    {
      std::lock_guard<std::mutex> const guard (_guard);

      auto pos_entry_point (_.find (entry_point));

      if (pos_entry_point == _.end())
      {
        return {};
      }

      auto pos_name (pos_entry_point->second.find (name));

      if (pos_name == pos_entry_point->second.end())
      {
        return {};
      }

      return pos_name->second;
    }
  }


namespace
{
  std::string build_parent_with_hostinfo
    (std::string const& name, fhg::drts::hostinfo_type const& hostinfo)
  {
    return fmt::format ( "{}%{}%{}"
                       , name
                       , hostinfo.first
                       , hostinfo.second
                       );
  }

  fhg::drts::hostinfo_type start_agent
    ( gspc::rif::entry_point const& rif_entry_point
    , gspc::rif::client& rif_client
    , std::string const& name
    , std::optional<unsigned short> const& agent_port
    , std::optional<std::filesystem::path> const& gpi_socket
    , gspc::installation_path const& installation_path
    , fhg::drts::processes_storage& processes
    , std::ostream& info_output
    , std::optional<std::pair<gspc::rif::client&, pid_t>> top_level_log
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
        , std::vector<gspc::logging::endpoint>
            {result.logger_registration_endpoint}
        ).get();
    }

    return result.hostinfo;
  }
}


  namespace fhg::drts
  {
    std::pair< std::unordered_set<gspc::rif::entry_point>
             , std::unordered_map<gspc::rif::entry_point, std::exception_ptr>
             > start_workers_for
    ( std::vector<gspc::rif::entry_point> const& entry_points
    , std::string parent_name
    , fhg::drts::hostinfo_type parent_hostinfo
    , gspc::worker_description const& description_
    , fhg::drts::processes_storage& processes
    , std::optional<std::filesystem::path> const& gpi_socket
    , std::vector<std::filesystem::path> const& app_path
    , std::vector<std::string> const& worker_env_copy_variable
    , bool worker_env_copy_current
    , std::vector<std::filesystem::path> const& worker_env_copy_file
    , std::vector<std::string> const& worker_env_set_variable
    , gspc::installation_path const& installation_path
    , std::ostream& info_output
    , std::optional<std::pair<gspc::rif::entry_point, pid_t>> top_level_log
    , gspc::Certificates const& certificates
    )
  {
     auto const& description (*description_._);

     std::string name_prefix (gspc::util::join (description.capabilities, '+').string());

     auto is_any_of
       { [] (std::string cs)
         {
           return [cs] (auto c)
             {
               return cs.find (c) != std::string::npos;
             };
         }
       };

     std::replace_if
       (name_prefix.begin(), name_prefix.end(), is_any_of ("+#.-"), '_');

     info_output << "I: starting " << name_prefix << " workers (parent "
                 << parent_name << ", "
                 << description.num_per_node << "/host, "
                 << ( description.max_nodes == 0 ? "unlimited"
                    : description.max_nodes == 1 ? "unique"
                    : "global max: " + std::to_string (description.max_nodes)
                    )
                 << ", " << description.shm_size << " SHM) with parent "
                 << parent_name << " on rif entry point "
                 << gspc::util::join
                      ( entry_points
                      , ", "
                      , [] ( std::ostream& os
                           , auto const& entry_point
                           ) -> std::ostream&
                        {
                          return os << entry_point.string();
                        }
                      )
                 << "\n";

     std::vector<std::string> arguments;

     arguments.emplace_back ("--parent");
     arguments.emplace_back
       (build_parent_with_hostinfo (parent_name, parent_hostinfo));

     for (auto const& path : app_path)
     {
       arguments.emplace_back ("--library-search-path");
       arguments.emplace_back (path.string());
     }

     if (description.shm_size)
     {
       arguments.emplace_back ("--capability");
       arguments.emplace_back ("GPI");
       arguments.emplace_back ("--virtual-memory-socket");
       arguments.emplace_back (gpi_socket->string());
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
       arguments.emplace_back (std::to_string (*description.socket));
     }

     if (certificates.path.has_value())
     {
       arguments.emplace_back ("--certificates");
       arguments.emplace_back (certificates.path->string());
     }

     std::atomic<std::size_t> num_nodes (0);

      std::unordered_map<gspc::rif::entry_point, std::vector<std::exception_ptr>>
        exceptions;

      //! \todo let thread count be a parameter
      gspc::util::scoped_boost_asio_io_service_with_threads io_service
        (std::max (1UL, std::min (64UL, entry_points.size())));

      std::list<std::pair<gspc::rif::client, gspc::rif::entry_point>> rif_connections;
      try
      {
        for (gspc::rif::entry_point const& entry_point : entry_points)
        {
          rif_connections.emplace_back
            ( std::piecewise_construct
            , std::forward_as_tuple (io_service, entry_point)
            , std::forward_as_tuple (entry_point)
            );
        }
      }
      catch (...)
      {
        std::throw_with_nested
          (std::runtime_error {"connecting to rif entry points"});
      }

      std::vector<std::tuple< gspc::rif::entry_point
                            , std::future<gspc::rif::protocol::start_worker_result>
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
                ? ("." + std::to_string (*description.socket))
                : std::string()
                )
              );
            std::string const storage_name ("drts-kernel-" + name);

            if ( auto const mpid
                   {processes.pidof (connection.second, storage_name)}
               )
            {
              throw std::logic_error
                ( "process with name '" + name + "' on entry point '"
                + connection.second.string() + "' already exists with pid "
                + std::to_string (*mpid)
                );
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
              auto const value (std::getenv (key.c_str()));
              if (!value)
              {
                throw std::invalid_argument
                  ( "requested to copy environment variable '" + key
                  + "', but variable is not set"
                  );
              }
              environment.emplace (key, value);
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
              for (auto const& definition : gspc::util::read_lines (file))
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

      std::vector<gspc::logging::endpoint> log_emitters;

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

      std::pair< std::unordered_set<gspc::rif::entry_point>
               , std::unordered_map<gspc::rif::entry_point, std::exception_ptr>
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
            gspc::util::throw_collected_exceptions (it->second);
          }
        }
        catch (...)
        {
          results.second.emplace (connection.second, std::current_exception());
        }
      }

      if (top_level_log)
      {
        gspc::rif::client (io_service, top_level_log->first)
          .add_emitter_to_logging_demultiplexer
            (top_level_log->second, log_emitters).get();
      }

      return results;
    }

    namespace
    {
      std::unique_ptr<gspc::rpc::remote_endpoint> connect
        (::boost::asio::io_service& io_service, gspc::logging::endpoint ep)
      {
        return gspc::util::visit
          ( ep.best (gspc::util::hostname())
          , [&] ( gspc::logging::socket_endpoint const& as_socket
                ) -> std::unique_ptr<gspc::rpc::remote_endpoint>
            {
              return std::make_unique<gspc::rpc::remote_socket_endpoint>
                (io_service, as_socket.socket);
            }
          , [&] ( gspc::logging::tcp_endpoint const& as_tcp
                ) -> std::unique_ptr<gspc::rpc::remote_endpoint>
            {
              return std::make_unique<gspc::rpc::remote_tcp_endpoint>
                (io_service, as_tcp);
            }
          );
      }
    }

    startup_result startup
      ( std::optional<unsigned short> const& agent_port
      , std::optional<std::filesystem::path> gpi_socket
      , gspc::installation_path const& installation_path
      , gspc::util::signal_handler_manager& signal_handler_manager
      , std::vector<gspc::rif::entry_point> const& rif_entry_points
      , gspc::rif::entry_point const& parent
      , fhg::drts::processes_storage& processes
      , std::string& parent_agent_name
      , fhg::drts::hostinfo_type& parent_agent_hostinfo
      , std::ostream& info_output
      , std::optional<gspc::rif::entry_point> log_rif_entry_point
      , std::vector<gspc::logging::endpoint> default_log_receivers
      , gspc::Certificates const& certificates
      )
    {
      gspc::util::scoped_signal_handler interrupt_signal_handler
        ( signal_handler_manager
        ,  SIGINT
        , [] (int, siginfo_t*, void*)
          {
            throw std::runtime_error ("Canceled...");
          }
        );

      info_output << "I: starting base scheduler components on " << parent << "...\n";

      //! \todo let thread count be a parameter
      gspc::util::scoped_boost_asio_io_service_with_threads io_service
        (std::max (1UL, std::min (64UL, rif_entry_points.size())));

      gspc::rif::client parent_rif_client (io_service, parent);

      std::optional<gspc::rif::client> logging_rif_client;
      std::optional<gspc::rif::protocol::start_logging_demultiplexer_result>
        logging_rif_info;
      if (log_rif_entry_point)
      {
        logging_rif_client.emplace
          (std::ref (io_service), *log_rif_entry_point);

        info_output << "I: starting top level gspc logging demultiplexer on "
                    << log_rif_entry_point->hostname << "\n";

        try
        {
          logging_rif_info
            = logging_rif_client->start_logging_demultiplexer
                (installation_path.logging_demultiplexer()).get();
        }
        catch (...)
        {
          std::throw_with_nested
            ( std::runtime_error
                {"Starting top level logging demultiplexer failed"}
            );
        }

        info_output << "   => accepting registration on '"
                    << logging_rif_info->sink_endpoint.to_string()
                    << "'\n";

        processes.store
          (*log_rif_entry_point, "logging-demultiplexer", logging_rif_info->pid);

        std::vector<gspc::logging::endpoint> const top_level_endpoint
          {logging_rif_info->sink_endpoint};
        for (auto const& receiver : default_log_receivers)
        {
          try
          {
            auto const endpoint (connect (io_service, receiver));
            using fun = gspc::logging::protocol::receiver::add_emitters;
            gspc::rpc::sync_remote_function<fun> {*endpoint} (top_level_endpoint);
          }
          catch (...)
          {
            std::throw_with_nested
              ( std::runtime_error
                 { "Requesting " + receiver.to_string()
                 + " to connect to top level logging demultiplexer failed"
                 }
              );
          }
        }
      }

      std::list<std::pair<gspc::rif::client, gspc::rif::entry_point>> rif_connections;
      std::vector<std::string> hostnames;
      try
      {
        for (gspc::rif::entry_point const& entry_point : rif_entry_points)
        {
          rif_connections.emplace_back
            ( std::piecewise_construct
            , std::forward_as_tuple (io_service, entry_point)
            , std::forward_as_tuple (entry_point)
            );
          hostnames.emplace_back (entry_point.hostname);
        }
      }
      catch (...)
      {
        std::throw_with_nested
          (std::runtime_error {"connecting to rif entry points"});
      }

      if (gpi_socket)
      {
        info_output << "I: external IML as VMEM running on: "
                    << gpi_socket.value()
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
          < gspc::rif::entry_point
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
          < gspc::rif::entry_point
          , std::pair< std::string /* kind */
                     , std::unordered_map<pid_t, std::exception_ptr>
                     >
          > failures;

        std::size_t processes_to_kill (0);
        for (auto const& entry_point_processes : entry_point_procs)
        {
          for (auto const& it : entry_point_processes->second)
          {
            if (gspc::util::starts_with (kind, it.first))
            {
              ++processes_to_kill;
            }
          }
        }

        //! \todo let thread count be a parameter
        gspc::util::scoped_boost_asio_io_service_with_threads io_service
          (std::min (64UL, processes_to_kill));

        using process_iter
          = typename decltype (entry_point_procs.front()->second)::iterator;

        std::list<gspc::rif::client> clients;
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
          for ( auto it (entry_point_processes->second.begin())
              ; it != entry_point_processes->second.end()
              ; ++it
              )
          {
            if (gspc::util::starts_with (kind, it->first))
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

                for (auto pid : pids)
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
                        << ": " << gspc::util::join (pids, ' ') << "\n";

            try
            {
              clients.emplace_back (io_service, entry_point);
              futures.emplace_back ( entry_point_processes
                                   , clients.back().kill (pids)
                                   , std::move (to_erase)
                                   , fail_group_with_current_exception
                                   );
            }
            catch (...) // \note: e.g. gspc::rif::client::connect
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

    std::unordered_map< gspc::rif::entry_point
                      , std::pair< std::string /* kind */
                                 , std::unordered_map<pid_t, std::exception_ptr>
                                 >
                      >
      processes_storage::shutdown_worker
        (std::vector<gspc::rif::entry_point> const& rif_entry_points)
    {
      std::vector<decltype (_)::iterator> iterators;
      for (gspc::rif::entry_point const& entry_point : rif_entry_points)
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
        < gspc::rif::entry_point
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
      gspc::util::apply_for_each_and_collect_exceptions
        ( { component_type::worker
          , component_type::agent
          , component_type::logging_demultiplexer
          }
        , [this] (component_type component)
          {
            std::unordered_map
              < gspc::rif::entry_point
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
                _info_output
                  << fmt::format
                     ( "Could not terminate {}[{}] on {}: {}"
                     , failure.second.first
                     , fails.first
                     , failure.first
                     , gspc::util::exception_printer (fails.second)
                     )
                  << std::endl
                  ;
              }
            }
          }
        );
    }
  }
