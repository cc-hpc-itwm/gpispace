// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <iml/detail/startup_and_shutdown.hpp>

#include <iml/rif/client.hpp>

#include <util-generic/functor_visitor.hpp>
#include <util-generic/hostname.hpp>
#include <util-generic/join.hpp>
#include <util-generic/make_optional.hpp>
#include <util-generic/nest_exceptions.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/read_file.hpp>
#include <util-generic/read_lines.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/serialization/boost/filesystem/path.hpp>
#include <util-generic/split.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <util-rpc/remote_function.hpp>
#include <util-rpc/remote_tcp_endpoint.hpp>
#include <util-rpc/remote_socket_endpoint.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/utility/in_place_factory.hpp>

#include <algorithm>
#include <cstddef>
#include <exception>
#include <functional>
#include <future>
#include <iterator>
#include <list>
#include <mutex>
#include <numeric>
#include <ostream>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace iml
{
    void RuntimeSystem::ProcessesStorage::store
      ( rif::EntryPoint const& entry_point
      , std::string const& name
      , pid_t pid
      )
    {
      std::lock_guard<std::mutex> const guard (_guard);

      if (!_[entry_point].emplace (name, pid).second)
      {
        throw std::logic_error
          ( str ( ::boost::format
                    ( "process with name '%1%' on entry point '%2%' already "
                      "exists with pid %3%, new pid %4%"
                    )
                % name
                % entry_point
                % _.at (entry_point).at (name)
                % pid
                )
          );
      }
    }
    ::boost::optional<pid_t> RuntimeSystem::ProcessesStorage::pidof
      (rif::EntryPoint const& entry_point, std::string const& name)
    {
      std::lock_guard<std::mutex> const guard (_guard);

      auto pos_entry_point (_.find (entry_point));

      if (pos_entry_point == _.end())
      {
        return ::boost::none;
      }

      auto pos_name (pos_entry_point->second.find (name));

      if (pos_name == pos_entry_point->second.end())
      {
        return ::boost::none;
      }

      return pos_name->second;
    }

    void RuntimeSystem::ProcessesStorage::startup
      ( ::boost::filesystem::path gpi_socket
      , std::chrono::seconds vmem_startup_timeout
      , unsigned short vmem_port
      , gaspi::NetdevID vmem_netdev_id
      , rif::EntryPoints const& rif_entry_points
      , std::ostream& info_output
      )
    {
      // \todo Ensure set of entry points is non-empty for sanity?

      //! \todo let thread count be a parameter
      fhg::util::scoped_boost_asio_io_service_with_threads io_service
        (std::max (1UL, std::min (64UL, rif_entry_points.size())));

      std::list<std::pair<fhg::iml::rif::client, rif::EntryPoint>> rif_connections;
      std::vector<std::string> hostnames;
      fhg::util::nest_exceptions<std::runtime_error>
        ( [&]
          {
            for ( rif::EntryPoint const& entry_point
                : rif_entry_points | ::boost::adaptors::map_values
                )
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

      info_output << "I: starting VMEM on: " << gpi_socket
                  << " with a timeout of " << vmem_startup_timeout.count()
                  << " seconds\n";

      fhg::util::nest_exceptions<std::runtime_error>
        ( [&]
          {
            std::unordered_map<rif::EntryPoint, std::future<pid_t>>
              queued_start_requests;
            std::unordered_map<rif::EntryPoint, std::exception_ptr>
              fails;

            //! \note requires ranks to be matching index in hostnames!
            std::size_t rank (0);
            for (auto& connection : rif_connections)
            {
              try
              {
                queued_start_requests.emplace
                  ( connection.second
                  , connection.first.start_vmem
                      ( gpi_socket
                      , vmem_port
                      , vmem_startup_timeout
                      , hostnames
                      , rank++
                      , vmem_netdev_id
                      )
                  );
              }
              catch (...)
              {
                fails.emplace (connection.second, std::current_exception());
              }
            }

            for (auto& request : queued_start_requests)
            {
              try
              {
                store (request.first, "vmem", request.second.get());
              }
              catch (...)
              {
                fails.emplace (request.first, std::current_exception());
              }
            }

            if (!fails.empty())
            {
              fhg::util::throw_collected_exceptions
                ( fails
                , [] (std::pair<rif::EntryPoint, std::exception_ptr> const& fail)
                  {
                    return ( ::boost::format ("vmem startup failed %1%: %2%")
                           % fail.first
                           % fhg::util::exception_printer (fail.second)
                           ).str();
                  }
                );
            }
          }
        , "could not start vmem"
        );
    }

    namespace
    {
      template<typename It>
        std::unordered_map
          < rif::EntryPoint
          , std::pair< std::string /* kind */
                     , std::unordered_map<pid_t, std::exception_ptr>
                     >
          > terminate_all_processes_of_a_kind
              ( std::vector<It> const& entry_point_procs
              , component_type component
              , std::ostream& info_output
              )
      {
        if (component != component_type::vmem)
        {
          throw std::logic_error ("invalid enum value");
        }

        std::string const kind ("vmem");

        std::unordered_map
          < rif::EntryPoint
          , std::pair< std::string /* kind */
                     , std::unordered_map<pid_t, std::exception_ptr>
                     >
          > failures;

        std::size_t processes_to_kill (0);
        for (auto const& entry_point_processes : entry_point_procs)
        {
          for (auto const& it : entry_point_processes->second)
          {
            if (::boost::algorithm::starts_with (it.first, kind))
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

        std::list<fhg::iml::rif::client> clients;
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
            if (::boost::algorithm::starts_with (it->first, kind))
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

      template<typename It>
        std::vector<It> iterators (It begin, It end)
      {
        std::vector<It> res (std::distance (begin, end));
        std::iota (res.begin(), res.end(), begin);
        return res;
      }
    }

    RuntimeSystem::ProcessesStorage::~ProcessesStorage()
    {
      fhg::util::apply_for_each_and_collect_exceptions
        ( { component_type::vmem
          }
        , [this] (component_type component)
          {
            std::unordered_map
              < rif::EntryPoint
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
                  ( ::boost::format ("Could not terminate %1%[%2%] on %3%: %4%")
                  % failure.second.first
                  % fails.first
                  % failure.first
                  % fhg::util::exception_printer (fails.second)
                  ) << std::endl;
              }
            }
          }
        );
    }
}
