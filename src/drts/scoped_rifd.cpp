// bernd.loerwald@itwm.fraunhofer.de

#include <drts/scoped_rifd.hpp>

#include <drts/drts.hpp>
#include <drts/private/pimpl.hpp>
#include <drts/private/option.hpp>
#include <drts/private/rifd_entry_points_impl.hpp>

#include <util-generic/read_lines.hpp>

#include <rif/strategy/meta.hpp>

#include <boost/format.hpp>
#include <boost/range/adaptor/map.hpp>

#include <algorithm>
#include <iterator>

namespace gspc
{
  namespace rifd
  {
    PIMPL_IMPLEMENTATION (strategy, std::string)
    PIMPL_IMPLEMENTATION (hostnames, std::vector<std::string>)
    PIMPL_IMPLEMENTATION (hostname, std::string)
    PIMPL_IMPLEMENTATION (port, boost::optional<unsigned short>)

    strategy::strategy (boost::program_options::variables_map const& vm)
      : _ (new implementation (require_rif_strategy (vm)))
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
    std::unordered_map<std::string, fhg::rif::entry_point>;

  namespace
  {
    template<typename L, typename R>
      struct by_first
    {
      bool operator() (std::pair<L,R> const& lhs, std::pair<L,R> const& rhs)
      {
        return lhs.first < rhs.first;
      }
    };

    entry_point_by_host set_difference ( entry_point_by_host const& lhs
                                       , entry_point_by_host const& rhs
                                       )
    {
      entry_point_by_host difference;

      std::set_difference
        ( lhs.begin(), lhs.end(), rhs.begin(), rhs.end()
        , std::inserter (difference, difference.begin())
        , by_first<std::string, fhg::rif::entry_point>()
        );

      return difference;
    }
  }

  struct rifds::implementation
  {
    implementation ( rifd::strategy const& strategy
                   , rifd::port const& port
                   , installation const& installation
                   )
      : _strategy (strategy._->_)
      , _port (port._->_)
      , _installation (installation)
    {}

    entry_point_by_host teardown (entry_point_by_host const& entry_points)
    {
      entry_point_by_host failed;

      fhg::rif::strategy::teardown (_strategy, entry_points, failed);

      _entry_points = set_difference
        (_entry_points, set_difference (entry_points, failed));

      return failed;
    }

    entry_point_by_host teardown()
    {
      return teardown (_entry_points);
    }

    std::string _strategy;
    boost::optional<unsigned short> _port;
    installation _installation;

    entry_point_by_host _entry_points;
  };

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

  rifds::rifds ( rifd::strategy const& strategy
               , rifd::port const& port
               , installation const& installation
               )
    : _ (new implementation (strategy, port, installation))
  {}
  PIMPL_DTOR (rifds)

  std::vector<std::string> rifds::hosts() const
  {
    return keys (_->_entry_points);
  }
  rifd_entry_points rifds::bootstrap (rifd::hostnames const& hostnames)
  {
    for (std::string const& hostname : hostnames._->_)
    {
      if (_->_entry_points.count (hostname))
      {
        throw std::invalid_argument
          ((boost::format ("boostrap: duplicate host %1%") % hostname).str());
      }
    }

    entry_point_by_host const new_entry_points
      ( fhg::rif::strategy::bootstrap
          (_->_strategy, hostnames._->_, _->_port, _->_installation.gspc_home())
      );

    for (auto new_entry_point : new_entry_points)
    {
      if (!_->_entry_points.emplace (new_entry_point).second)
      {
        throw std::logic_error
          (( boost::format ("STRANGE: duplicate key '%1%'!?")
           % new_entry_point.first
           ).str()
          );
      }
    }

    return new rifd_entry_points::implementation (values (new_entry_points));
  }
  std::vector<std::string> rifds::teardown (rifd::hostnames const& hostnames)
  {
    entry_point_by_host entry_points_to_remove;

    for (std::string const& hostname : hostnames._->_)
    {
      auto pos (_->_entry_points.find (hostname));

      if (pos == _->_entry_points.end())
      {
        throw std::invalid_argument
          ((boost::format ("teardown: unknown host '%1%'") % hostname).str());
      }

      entry_points_to_remove.emplace (*pos);
    }

    return keys (_->teardown (entry_points_to_remove));
  }
  std::vector<std::string> rifds::teardown()
  {
    return keys (_->teardown());
  }

  scoped_rifds::scoped_rifds ( rifd::strategy const& strategy
                             , rifd::hostnames const& hostnames
                             , rifd::port const& port
                             , installation const& installation
                             )
    : rifds (strategy, port, installation)
  {
    bootstrap (hostnames);
  }
  scoped_rifds::~scoped_rifds()
  {
    teardown();
  }
  rifd_entry_points scoped_rifds::entry_points() const
  {
    return new rifd_entry_points::implementation
      (_->_entry_points | boost::adaptors::map_values);
  }

  scoped_rifd::scoped_rifd ( rifd::strategy const& strategy
                           , rifd::hostname const& hostname
                           , rifd::port const& port
                           , installation const& installation
                           )
    : rifds (strategy, port, installation)
  {
    bootstrap (std::vector<std::string> {hostname._->_});
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
}
