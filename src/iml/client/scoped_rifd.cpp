#include <iml/client/scoped_rifd.hpp>

#include <iml/client/iml.hpp>
#include <iml/client/private/pimpl.hpp>
#include <iml/client/private/option.hpp>
#include <iml/client/private/rifd_entry_points_impl.hpp>

#include <util-generic/blocked.hpp>
#include <util-generic/read_lines.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <iml/rif/client.hpp>
#include <iml/rif/strategy/meta.hpp>

#include <boost/format.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/serialization/unordered_map.hpp>

#include <iml/rif/client.hpp>

#include <iterator>

namespace iml_client
{
  namespace iml_rifd
  {
    PIMPL_IMPLEMENTATION
      (strategy, std::pair<std::string, std::vector<std::string>>)
    PIMPL_IMPLEMENTATION (hostnames, std::vector<std::string>)
    PIMPL_IMPLEMENTATION (hostname, std::string)
    PIMPL_IMPLEMENTATION (port, boost::optional<unsigned short>)

    strategy::strategy (boost::program_options::variables_map const& vm)
      : _ ( new implementation
              (  std::make_pair ( require_rif_strategy (vm)
                                , require_rif_strategy_parameters (vm)
                                )
              )
          )
    {}

    hostnames::hostnames (boost::program_options::variables_map const& vm)
      : _ (new implementation (fhg::util::read_lines (require_nodefile (vm))))
    {}
    hostnames::hostnames (std::vector<std::string> const& hostnames)
      : _ (new implementation (hostnames))
    {}
    hostname::hostname (std::string const& host)
      : _ (new implementation (host))
    {}

    port::port (boost::program_options::variables_map const& vm)
      : _ (new implementation (get_rif_port (vm)))
    {}
  }

  using entry_point_by_host =
    std::unordered_map<std::string, fhg::iml::rif::entry_point>;

  struct rifds::implementation
  {
    implementation ( iml_rifd::strategy const& strategy
                   , iml_rifd::port const& port
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
                  (( boost::format ("bootstrap: duplicate host %1%")
                   % hostname
                   ).str()
                  )
              )
            );
        }
      }

      std::tuple< entry_point_by_host
                , std::unordered_map<std::string, std::exception_ptr>
                , std::unordered_map<std::string, std::string>
                > const boot
        ( fhg::iml::rif::strategy::bootstrap ( _strategy
                                             , no_duplicates
                                             , _port
                                             , _installation.iml_home()
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
            (( boost::format ("STRANGE: duplicate key '%1%'!?")
             % new_entry_point.first
             ).str()
            );
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
        (fhg::iml::rif::strategy::teardown (_strategy, entry_points, _parameters));

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

    std::string _strategy;
    std::vector<std::string> _parameters;
    boost::optional<unsigned short> _port;
    installation _installation;

    entry_point_by_host _entry_points;
    std::unordered_map<std::string, std::string> _real_hostnames;
  };

  rifds::rifds ( iml_rifd::strategy const& strategy
               , iml_rifd::port const& port
               , installation const& installation
               )
    : _ (new implementation (strategy, port, installation))
  {}
  PIMPL_DTOR (rifds)

  namespace
  {
    template<typename Key, typename Value>
      std::vector<Key> keys (std::unordered_map<Key, Value> const& map)
    {
      auto range (map | boost::adaptors::map_keys);

      return {std::begin (range), std::end (range)};
    }

    template<typename Key, typename Value>
      std::vector<Value> values (std::unordered_map<Key, Value> const& map)
    {
      auto range (map | boost::adaptors::map_values);

      return {std::begin (range), std::end (range)};
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
           > rifds::entry_points (iml_rifd::hostnames const& hostnames) const
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
    rifds::bootstrap (iml_rifd::hostnames const& hostnames, std::ostream& out)
  {
    auto result (_->bootstrap (hostnames._->_, out));

    return { new rifd_entry_points::implementation (values (result.first))
           , result.second
           };
  }

  std::pair < std::unordered_set<std::string>
            , std::unordered_map<std::string, std::exception_ptr>
            >
    rifds::teardown (iml_rifd::hostnames const& hostnames)
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

  scoped_rifd::scoped_rifd ( iml_rifd::strategy const& strategy
                           , iml_rifd::hostname const& hostname
                           , iml_rifd::port const& port
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

  scoped_rifds::scoped_rifds ( iml_rifd::strategy const& strategy
                             , iml_rifd::hostnames const& hostnames
                             , iml_rifd::port const& port
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