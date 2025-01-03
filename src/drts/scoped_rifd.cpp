// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <drts/scoped_rifd.hpp>

#include <drts/drts.hpp>
#include <drts/private/option.hpp>
#include <drts/private/pimpl.hpp>
#include <drts/private/rifd_entry_points_impl.hpp>

#include <util-generic/read_lines.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <rif/client.hpp>
#include <rif/strategy/meta.hpp>

#include <boost/serialization/unordered_map.hpp>

#include <algorithm>
#include <fmt/core.h>
#include <iterator>

namespace gspc
{
  namespace rifd
  {
    PIMPL_IMPLEMENTATION
      (strategy, std::pair<std::string, std::vector<std::string>>)
    PIMPL_IMPLEMENTATION (hostnames, std::vector<std::string>)
    PIMPL_IMPLEMENTATION (hostname, std::string)
    PIMPL_IMPLEMENTATION (port, ::boost::optional<unsigned short>)

    strategy::strategy (::boost::program_options::variables_map const& vm)
      : _ ( new implementation
              (  std::make_pair ( require_rif_strategy (vm)
                                , require_rif_strategy_parameters (vm)
                                )
              )
          )
    {}

    hostnames::hostnames (::boost::program_options::variables_map const& vm)
      : _ (new implementation (fhg::util::read_lines (require_nodefile (vm))))
    {}
    hostnames::hostnames (std::vector<std::string> const& hostnames_)
      : _ (new implementation (hostnames_))
    {}
    hostname::hostname (std::string const& host)
      : _ (new implementation (host))
    {}

    port::port (::boost::program_options::variables_map const& vm)
      : _ (new implementation (get_rif_port (vm)))
    {}
  }

  using entry_point_by_host =
    std::unordered_map<std::string, fhg::rif::entry_point>;

  struct rifds::implementation
  {
    implementation ( rifd::strategy const& strategy
                   , rifd::port const& port
                   , installation const& installation
                   )
      : _strategy (strategy._->_.first)
      , _parameters (strategy._->_.second)
      , _port (port._->_)
      , _installation (installation)
    {}

    std::pair< entry_point_by_host
             , std::pair< std::unordered_set<std::string>
                        , std::unordered_set<std::string>
                        >
             > entry_points
      (std::vector<std::string> const& hostnames) const
    {
      entry_point_by_host entry_points;
      std::unordered_set<std::string> known;
      std::unordered_set<std::string> unknown;

      for (std::string const& hostname : hostnames)
      {
        auto entry_point (_entry_points.find (hostname));

        if (entry_point == _entry_points.end())
        {
          unknown.emplace (hostname);
        }
        else
        {
          known.emplace (hostname);
          entry_points.emplace (*entry_point);
        }
      }

      return {entry_points, {known, unknown}};
    }
    entry_point_by_host const& entry_points() const
    {
      return _entry_points;
    }

    std::pair< entry_point_by_host
             , std::unordered_map<std::string, std::exception_ptr>
             >
      bootstrap (std::vector<std::string> const& hostnames, std::ostream& out)
    {
      std::unordered_map<std::string, std::exception_ptr> failed;
      std::vector<std::string> no_duplicates;

      for (std::string const& hostname : hostnames)
      {
        if (!_entry_points.count (hostname))
        {
          no_duplicates.emplace_back (hostname);
        }
        else
        {
          failed.emplace
            ( hostname
            , std::make_exception_ptr
              ( std::invalid_argument
                  { fmt::format ("bootstrap: duplicate host {}", hostname)
                  }
              )
            );
        }
      }

      std::tuple< entry_point_by_host
                , std::unordered_map<std::string, std::exception_ptr>
                , std::unordered_map<std::string, std::string>
                > const boot
        ( fhg::rif::strategy::bootstrap
          ( _strategy
          , no_duplicates
          , _port
          , ::boost::filesystem::path {_installation.gspc_home().string()}
          , _parameters
          , out
          )
        );

      std::unordered_map<std::string, std::string> const& real_hostname
        (std::get<2> (boot));

      for (auto const& new_entry_point : std::get<0> (boot))
      {
        if (!_entry_points.emplace (new_entry_point).second)
        {
          throw std::logic_error
            { fmt::format
                ( "STRANGE: duplicate key '{}'!?"
                , new_entry_point.first
                )
            };
        }

        _real_hostnames.emplace
          (new_entry_point.first, real_hostname.at (new_entry_point.first));
      }

      for (auto& failed_boot : std::get<1> (boot))
      {
        failed.emplace (std::move (failed_boot));
      }

      return {std::get<0> (boot), failed};
    }

    std::pair < std::unordered_set<std::string>
              , std::unordered_map<std::string, std::exception_ptr>
              > teardown (entry_point_by_host const entry_points)
    {
      auto const result
        (fhg::rif::strategy::teardown (_strategy, entry_points, _parameters));

      for (auto const& entry_point : entry_points)
      {
        _entry_points.erase (entry_point.first);
        _real_hostnames.erase (entry_point.first);
      }

      return result;
    }

    std::pair < std::unordered_set<std::string>
              , std::unordered_map<std::string, std::exception_ptr>
              > teardown()
    {
      return teardown (_entry_points);
    }

    std::pair< std::unordered_map<std::string, std::vector<std::string>>
             , std::unordered_map<std::string, std::exception_ptr>
             >
      execute ( std::unordered_set<std::string> const& hostnames
              , ::boost::filesystem::path const& command
              , std::vector<std::string> const& arguments
              , std::unordered_map<std::string, std::string> const& environment
              ) const
    {
      std::pair< std::unordered_map<std::string, std::vector<std::string>>
               , std::unordered_map<std::string, std::exception_ptr>
               > results;

      fhg::util::scoped_boost_asio_io_service_with_threads io_service (64);

      std::list<fhg::rif::client> clients;
      std::unordered_map<std::string, std::future<std::vector<std::string>>> futures;
      for (std::string const& hostname : hostnames)
      {
        try
        {
          auto const pos (_entry_points.find (hostname));

          if (pos == _entry_points.end())
          {
            throw std::invalid_argument
              {fmt::format ("execute: unknown host '{}'", hostname)};
          }

          clients.emplace_back (io_service, pos->second);
          futures.emplace
            ( hostname
            , clients.back().execute_and_get_startup_messages_and_wait
                (command, arguments, environment)
            );
        }
        catch (...)
        {
          results.second.emplace (hostname, std::current_exception());
        }
      }

      for (auto& future : futures)
      {
        try
        {
          results.first.emplace (future.first, future.second.get());
        }
        catch (...)
        {
          results.second.emplace (future.first, std::current_exception());
        }
      }

      return results;
    }

    std::string _strategy;
    std::vector<std::string> _parameters;
    ::boost::optional<unsigned short> _port;
    installation _installation;

    entry_point_by_host _entry_points;
    std::unordered_map<std::string, std::string> _real_hostnames;
  };

  rifds::rifds ( rifd::strategy const& strategy
               , rifd::port const& port
               , installation const& installation
               )
    : _ (new implementation (strategy, port, installation))
  {}
  PIMPL_DTOR (rifds)

  namespace
  {
    template<typename Key, typename Value>
      std::vector<Value> values (std::unordered_map<Key, Value> const& map)
    {
      auto vs {std::vector<Value>{}};
      vs.reserve (map.size());
      std::transform
        ( std::begin (map), std::end (map)
        , std::back_inserter (vs)
        , [] (auto const& kv)
          {
            auto const& [key, value] {kv};
            return value;
          }
        );
      return vs;
    }
  }

  std::vector<std::string> rifds::hosts() const
  {
    return values (_->_real_hostnames);
  }
  std::pair< rifd_entry_points
           , std::pair< std::unordered_set<std::string>
                      , std::unordered_set<std::string>
                      >
           > rifds::entry_points (rifd::hostnames const& hostnames) const
  {
    auto const entry_points (_->entry_points (hostnames._->_));

    return { new rifd_entry_points::implementation (values (entry_points.first))
           , entry_points.second
           };
  }
  rifd_entry_points rifds::entry_points() const
  {
    return new rifd_entry_points::implementation (values (_->entry_points()));
  }

  std::pair< rifd_entry_points
           , std::unordered_map<std::string, std::exception_ptr>
           >
    rifds::bootstrap (rifd::hostnames const& hostnames, std::ostream& out)
  {
    auto result (_->bootstrap (hostnames._->_, out));

    return { new rifd_entry_points::implementation (values (result.first))
           , result.second
           };
  }

  std::pair < std::unordered_set<std::string>
            , std::unordered_map<std::string, std::exception_ptr>
            >
    rifds::teardown (rifd::hostnames const& hostnames)
  {
    return _->teardown (_->entry_points (hostnames._->_).first);
  }

  std::pair < std::unordered_set<std::string>
            , std::unordered_map<std::string, std::exception_ptr>
            >
    rifds::teardown()
  {
    return _->teardown();
  }

  std::pair< std::unordered_map<std::string, std::vector<std::string>>
           , std::unordered_map<std::string, std::exception_ptr>
           >
    rifds::execute
      ( std::unordered_set<std::string> const& hostnames
      , ::boost::filesystem::path const& command
      , std::vector<std::string> const& arguments
      , std::unordered_map<std::string, std::string> const& environment
      ) const
  {
    return _->execute (hostnames, command, arguments, environment);
  }

  scoped_rifd::scoped_rifd ( rifd::strategy const& strategy
                           , rifd::hostname const& hostname
                           , rifd::port const& port
                           , installation const& installation
                           , std::ostream& out
                           )
    : rifds (strategy, port, installation)
  {
    auto const failed
      (bootstrap (std::vector<std::string> {hostname._->_}, out).second);
    if (!failed.empty())
    {
      teardown();

      fhg::util::throw_collected_exceptions (values (failed));
    }
  }
  scoped_rifd::~scoped_rifd()
  {
    teardown();
  }
  rifd_entry_point scoped_rifd::entry_point() const
  {
    return new rifd_entry_point::implementation
      (_->_entry_points.begin()->second);
  }

  scoped_rifds::scoped_rifds ( rifd::strategy const& strategy
                             , rifd::hostnames const& hostnames
                             , rifd::port const& port
                             , installation const& installation
                             , std::ostream& out
                             )
    : rifds (strategy, port, installation)
  {
    auto const failed (bootstrap (hostnames, out).second);
    if (!failed.empty())
    {
      teardown();

      fhg::util::throw_collected_exceptions (values (failed));
    }
  }
  scoped_rifds::~scoped_rifds()
  {
    teardown();
  }
}
