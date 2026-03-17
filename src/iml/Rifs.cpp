// Copyright (C) 2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/iml/Rifs.hpp>

#include <gspc/iml/detail/Installation.hpp>
#include <gspc/iml/detail/option.hpp>
#include <gspc/rif/strategy/meta.hpp>

#include <gspc/util/boost/program_options/validators/existing_path.hpp>
#include <gspc/util/boost/program_options/validators/positive_integral.hpp>
#include <gspc/util/join.hpp>
#include <gspc/util/read_lines.hpp>
#include <gspc/util/wait_and_collect_exceptions.hpp>

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace gspc::iml
{
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

  Rifs::Rifs ( std::vector<std::string> const& hostnames
             , std::string const& strategy
             , std::vector<std::string> const& strategy_parameters
             , std::optional<unsigned short> const& port
             , std::ostream& out
             )
    : _strategy (strategy)
    , _strategy_parameters (strategy_parameters)
    , _port (port)
  {
    detail::Installation const installation;

    auto const& [entry_points, failures, messages]
      { gspc::rif::strategy::bootstrap
          ( _strategy
          , hostnames
          , _port
          , installation.installation_prefix
          , _strategy_parameters
          , out
          )
      };

    _entry_points = entry_points;

    if (!failures.empty())
    {
      gspc::rif::strategy::teardown
        (_strategy, _entry_points, _strategy_parameters);

      gspc::util::throw_collected_exceptions (values (failures));
    }
  }

  Rifs::~Rifs()
  {
    gspc::rif::strategy::teardown
      (_strategy, _entry_points, _strategy_parameters);
  }

  Rifs::Rifs ( rif::EntryPoints entry_points
             , std::string strategy
             , std::vector<std::string> strategy_parameters
             )
    : _strategy (std::move (strategy))
    , _strategy_parameters (std::move (strategy_parameters))
    , _entry_points (std::move (entry_points))
  {}

  Rifs::operator rif::EntryPoints() const
  {
    return _entry_points;
  }

  namespace
  {
    namespace option
    {
      namespace validators = gspc::util::boost::program_options;

      constexpr auto const name_nodefile ("iml-rif-nodefile");
      using type_nodefile = std::filesystem::path;
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
    : Rifs ( gspc::util::read_lines
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
        + gspc::util::join
            (gspc::rif::strategy::available_strategies(), ", ").string()
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
          return gspc::util::join (x, ", ").string();
        }
      );
  }
}
