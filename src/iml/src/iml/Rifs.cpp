// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/Rifs.hpp>

#include <iml/detail/option.hpp>
#include <iml/rif/bootstrap.hpp>
#include <iml/rif/strategy/meta.hpp>
#include <iml/rif/teardown.hpp>

#include <util-generic/boost/program_options/validators/existing_path.hpp>
#include <util-generic/boost/program_options/validators/positive_integral.hpp>
#include <util-generic/join.hpp>
#include <util-generic/read_lines.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <boost/range/adaptor/map.hpp>

#include <iterator>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace iml
{
  namespace
  {
    template<typename Key, typename Value>
      std::vector<Value> values (std::unordered_map<Key, Value> const& map)
    {
      auto range (map | ::boost::adaptors::map_values);

      return {std::begin (range), std::end (range)};
    }
  }

  Rifs::Rifs ( std::vector<std::string> const& hostnames
             , std::string const& strategy
             , std::vector<std::string> const& strategy_parameters
             , ::boost::optional<unsigned short> const& port
             , std::ostream& out
             )
    : _strategy (strategy)
    , _strategy_parameters (strategy_parameters)
    , _port (port)
  {
    auto const boot
      ( iml::rif::bootstrap
          (hostnames, _strategy, _strategy_parameters, _port, out)
      );

    _entry_points = std::move (boot.entry_points);

    if (!boot.failures_by_host.empty())
    {
      iml::rif::teardown (_entry_points, _strategy, _strategy_parameters);

      fhg::util::throw_collected_exceptions (values (boot.failures_by_host));
    }
  }

  Rifs::~Rifs()
  {
    iml::rif::teardown (_entry_points, _strategy, _strategy_parameters);
  }

  Rifs::operator rif::EntryPoints() const
  {
    return _entry_points;
  }

  namespace
  {
    namespace option
    {
      namespace validators = fhg::util::boost::program_options;

      constexpr auto const name_nodefile ("iml-rif-nodefile");
      using type_nodefile = ::boost::filesystem::path;
      using validator_nodefile = validators::existing_path;
      constexpr auto const name_port ("iml-rif-port");
      using type_port = unsigned short;
      using validator_port = type_port;
      constexpr auto const name_strategy ("iml-rif-strategy");
      using type_strategy = std::string;
      using validator_strategy = type_strategy;
      constexpr auto const name_strategy_parameters
        ("iml-rif-strategy-parameters");
      using type_strategy_parameters = std::vector<std::string>;
      using validator_strategy_parameters = type_strategy_parameters;
    }
  }

  Rifs::Rifs ( ::boost::program_options::variables_map const& vm
             , std::ostream& output
             )
    : Rifs ( fhg::util::read_lines
               ( detail::require<option::type_nodefile, option::validator_nodefile>
                   (vm, option::name_nodefile)
               )
           , detail::require<option::type_strategy, option::validator_strategy>
               (vm, option::name_strategy)
           , detail::require<option::type_strategy_parameters, option::validator_strategy_parameters>
               (vm, option::name_strategy_parameters)
           , detail::get<option::type_port, option::validator_port>
               (vm, option::name_port)
           , output
           )
  {}

  ::boost::program_options::options_description Rifs::options()
  {
    ::boost::program_options::options_description options
      ("Remote Interface Daemon (internally started)");

    options.add_options()
      ( option::name_nodefile
      , ::boost::program_options::value<option::validator_nodefile>()
        ->required()
      , "nodefile for rifds to be started on"
      );

    options.add_options()
      ( option::name_strategy
      , ::boost::program_options::value<option::validator_strategy>()
        ->required()
      , ( "strategy used to bootstrap rifd (one of "
        + fhg::util::join
            (fhg::iml::rif::strategy::available_strategies(), ", ").string()
        + ")"
        ).c_str()
      );
    options.add_options()
      ( option::name_strategy_parameters
      , ::boost::program_options::value<option::validator_strategy_parameters>()
        ->default_value (option::type_strategy_parameters(), "")
        ->required()
      , "parameters passed to bootstrapping strategy"
      );

    options.add_options()
      ( option::name_port
      , ::boost::program_options::value<option::validator_port>()
      , "port for rifd to listen on"
      );

    return options;
  }

  void Rifs::set_nodefile
    (::boost::program_options::variables_map& vm, option::type_nodefile value)
  {
    detail::set<option::type_nodefile, option::validator_nodefile>
      ( vm, option::name_nodefile, value
      , [] (option::type_nodefile const& x)
        {
          return x.string();
        }
      );
  }

  void Rifs::set_port
    (::boost::program_options::variables_map& vm, option::type_port value)
  {
    detail::set<option::type_port, option::validator_port>
      ( vm, option::name_port, value
      , [] (option::type_port const& x)
        {
          return std::to_string (x);
        }
      );
  }

  void Rifs::set_strategy
    (::boost::program_options::variables_map& vm, option::type_strategy value)
  {
    detail::set<option::type_strategy, option::validator_strategy>
      ( vm, option::name_strategy, value
      , [] (option::type_strategy const& x)
        {
          return x;
        }
      );
  }

  void Rifs::set_strategy_parameters
    ( ::boost::program_options::variables_map& vm
    , option::type_strategy_parameters value
    )
  {
    detail::set<option::type_strategy_parameters, option::validator_strategy_parameters>
      ( vm, option::name_strategy_parameters, value
      , [] (option::type_strategy_parameters const& x)
        {
          return fhg::util::join (x, ", ").string();
        }
      );
  }
}
