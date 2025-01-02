// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <bin/run.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>
#include <we/type/value/show.hpp>

#include <boost/lexical_cast.hpp>

#include <exception>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/std.h>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace fmt
{
  template<>
    struct formatter<pnet::type::value::show> : fmt::ostream_formatter
  {};
}

namespace stochastic_with_heureka
{
  namespace
  {
    struct existing_path_in_installation
    {
      existing_path_in_installation (std::filesystem::path const& path)
        : _path {path}
      {
        if (!std::filesystem::exists (_path))
        {
          throw std::logic_error
            { fmt::format
                ( "'{}' does not exist: Installation incomplete!?"
                , _path
                )
            };
        }
      }
      operator std::filesystem::path() const
      {
        return _path;
      }

    private:
      std::filesystem::path _path;
    };

    struct existing_path
    {
      existing_path (std::filesystem::path const& path)
        : _path {path}
      {
        if (!std::filesystem::exists (_path))
        {
          throw std::logic_error
            { fmt::format ( "'{}' does not exist"
                          , _path
                          )
            };
        }
      }

      operator std::filesystem::path() const
      {
        return _path;
      }

    private:
      std::filesystem::path _path;
    };

    template<typename T>
      T extract
        ( std::multimap<std::string, pnet::type::value::value_type> const& r
        , std::string const& key
        )
    {
      if (r.count (key) != 1)
      {
        std::ostringstream oss;
        bool first (true);

        oss << "{";
        for (auto const& x : r)
        {
          if (!first)
          {
            oss << ", ";
          }
          else
          {
            first = false;
          }
          oss << "'" << x.first << "': " << pnet::type::value::show (x.second);
        }
        oss << "}";

        throw std::runtime_error
          { fmt::format
              ( "Extract: {} key '{}' in '{}'"
              , (r.count (key) ? "Duplicate" : "Missing")
              , key
              , oss.str()
              )
          };
      }

      auto const& value (r.find (key)->second);

      try
      {
        return ::boost::get<T> (value);
      }
      catch (...)
      {
        std::throw_with_nested
          ( std::runtime_error
              { fmt::format ( "Extract: Key '{}', value '{}'"
                            , key
                            , pnet::type::value::show (value)
                            )
              }
          );
      }

      __builtin_unreachable();
    }

    namespace option_name
    {
      constexpr char const* const help {"help"};
      constexpr char const* const user_data {"user-data"};
      constexpr char const* const number_of_rolls {"number-of-rolls"};
      constexpr char const* const seed {"seed"};
      constexpr char const* const rolls_at_once {"rolls-at-once"};
      constexpr char const* const roll_and_heureka_per_node
        {"roll-and-heureka-per-node"};
      constexpr char const* const reduce_per_node {"reduce-per-node"};
      constexpr char const* const post_process_per_node {"post-process-per-node"};
    }

    struct positive_unsigned_long
    {
      positive_unsigned_long (std::string const& option)
        : positive_unsigned_long (std::stoul (option))
      {}
      positive_unsigned_long (unsigned long value)
        : _value (value)
      {
        if (!(_value > 0))
        {
          throw ::boost::program_options::invalid_option_value
            {fmt::format ("'{}' not positive.", _value)};
        }
      }
      operator unsigned long const& () const
      {
        return _value;
      }

    private:
      unsigned long _value;
    };

    void validate ( ::boost::any& v
                  , std::vector<std::string> const& values
                  , positive_unsigned_long*
                  , int
                  )
    {
      ::boost::program_options::validators::check_first_occurrence (v);
      auto const value
        (::boost::program_options::validators::get_single_string (values));
      v = ::boost::any (positive_unsigned_long (value));
    }
  }

  workflow_result::workflow_result
      (std::multimap<std::string, pnet::type::value::value_type> const& r)
    : _result (extract<we::type::bytearray> (r, "result"))
    , _got_heureka (extract<bool> (r, "got_heureka"))
    , _number_of_rolls_done (extract<unsigned long> (r, "number_of_rolls_done"))
  {}

  workflow_result run
    ( int argc
    , char** argv
    , std::optional<::boost::program_options::options_description> options
    , std::string const implementation
    , std::function< we::type::bytearray
                       (::boost::program_options::variables_map const&)
                   > user_data
    )
  {
    return run ( argc
               , argv
               , options
               , [implementation] ( ::boost::program_options::variables_map const&
                                  , std::filesystem::path installation_dir
                                  )
                 {
                   return static_cast<std::filesystem::path>
                     ( existing_path_in_installation
                       ( installation_dir
                       / "implementation"
                       / ("lib" + implementation + ".so")
                       )
                     );
                 }
               , user_data
               );
  }

  workflow_result run
    ( int argc
    , char** argv
    , std::optional<::boost::program_options::options_description>
        implementation_options
    , std::function< std::filesystem::path
                       ( ::boost::program_options::variables_map const&
                       , std::filesystem::path installation_dir
                       )
                   > implementation
    , std::function< we::type::bytearray
                     (::boost::program_options::variables_map const&)
                   > implementation_bytearray
    )
  {
    std::filesystem::path const binary_path
      (std::filesystem::canonical (argv[0]).parent_path());
    std::filesystem::path const workflow_path
      (binary_path.parent_path() / "workflow");

    std::filesystem::path const gspc_home
#if defined SWH_DEPLOYMENT_STRATEGY_InstallationCanNotBeMovedAndRefersToDependenciesUsingAbsolutePaths
      (GSPC_HOME);
#elif defined SWH_DEPLOYMENT_STRATEGY_LocationAndHostIndependentBundle
      ( binary_path.parent_path()
      / "libexec" / "bundle" / "gpispace"
      );
#endif

    if (!std::filesystem::is_directory (workflow_path))
    {
      throw std::logic_error
        { fmt::format ( "'{}' is not a directory: Installation incomplete!?"
                      , workflow_path
                      )
        };
    }

    existing_path_in_installation const workflow
      (workflow_path / "stochastic_with_heureka.pnet");

    ::boost::program_options::options_description general ("General");

    general.add_options()
      (option_name::help, "this message")
      ;

    general.add (gspc::options::installation());
    general.add (gspc::options::drts());
    general.add (gspc::options::logging());
    general.add (gspc::options::scoped_rifd());

    ::boost::program_options::options_description job ("Job");

    job.add_options()
      ( option_name::number_of_rolls
      , ::boost::program_options::value<positive_unsigned_long>()->required()
      , "overall number of dice rolls"
      )
      ( option_name::seed
      , ::boost::program_options::value<unsigned long>()->required()
      , "initial seed, subsequent seed are calculated by seed' = seed + 1"
      )
      ;

    ::boost::program_options::options_description parallelism ("Parallelism");

    parallelism.add_options()
      ( option_name::rolls_at_once
      , ::boost::program_options::value<positive_unsigned_long>()->required()
      , "number of dice rolls in a single bunch"
      )
      ( option_name::roll_and_heureka_per_node
      , ::boost::program_options::value<positive_unsigned_long>()->required()
      , "number of roll-and-reduce worker per node"
      )
      ( option_name::reduce_per_node
      , ::boost::program_options::value<positive_unsigned_long>()->required()
      , "number of reduce worker per node"
      )
      ( option_name::post_process_per_node
      , ::boost::program_options::value<positive_unsigned_long>()->required()
      , "number of post-process worker per node"
      )
      ;

    ::boost::program_options::options_description options_description;

    options_description.add (general);
    options_description.add (job);
    options_description.add (parallelism);

    if (implementation_options)
    {
      options_description.add (*implementation_options);
    }

    ::boost::program_options::variables_map vm;
    gspc::set_gspc_home (vm, gspc_home);

    ::boost::program_options::store
      ( ::boost::program_options::command_line_parser (argc, argv)
      . options (options_description).run()
      , vm
      );

    if (vm.count (option_name::help))
    {
      throw std::runtime_error
        (::boost::lexical_cast<std::string> (options_description));
    }

    vm.notify();

    existing_path const implementation_so
      (implementation (vm, binary_path.parent_path()));

    unsigned long const number_of_rolls
      ( vm[option_name::number_of_rolls]
      . as<positive_unsigned_long>()
      );
    unsigned long const rolls_at_once
      ( vm[option_name::rolls_at_once]
      . as<positive_unsigned_long>()
      );
    unsigned long const roll_and_heureka_per_node
      ( vm[option_name::roll_and_heureka_per_node]
      . as<positive_unsigned_long>()
      );
    unsigned long const seed (vm[option_name::seed].as<unsigned long>());

    gspc::set_application_search_path (vm, workflow_path);

    we::type::bytearray const user_data (implementation_bytearray (vm));

    auto worker
      ( [&vm] (std::string const& name, char const* const option) -> std::string
        {
          return fmt::format
            ( "{}:{},0"
            , name
            , static_cast<unsigned long>
                (vm[option].as<positive_unsigned_long>())
            );
        }
      );

    std::ostringstream topology_description;

    topology_description
      << worker ("roll_and_heureka", option_name::roll_and_heureka_per_node)
      << " " << worker ("reduce", option_name::reduce_per_node)
      << " " << worker ("post_process", option_name::post_process_per_node)
      ;

    gspc::installation const installation (vm);
    gspc::scoped_rifds const rifds { gspc::rifd::strategy (vm)
                                   , gspc::rifd::hostnames (vm)
                                   , gspc::rifd::port (vm)
                                   , installation
                                   };
    gspc::scoped_runtime_system const drts
      (vm, installation, topology_description.str(), rifds.entry_points());

    return
      { gspc::client (drts).put_and_run
        ( static_cast<std::filesystem::path> (workflow)
        , { {"number_of_rolls", number_of_rolls}
          , {"rolls_at_once", rolls_at_once}
          , {"seed", seed}
          , {"implementation", static_cast<std::filesystem::path> (implementation_so).string()}
          , {"user_data", user_data}
          , { "parallel_rolls"
            , roll_and_heureka_per_node * rifds.hosts().size()
            }
          }
        )
      };
  }
}
