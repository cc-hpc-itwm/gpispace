// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <bin/run.hpp>
#include <util/print_exception.hpp>

#include <implementation/finance/barropt.hpp>

#include <boost/format.hpp>

#include <exception>
#include <ios>
#include <iostream>
#include <ostream>
#include <string>
#include <utility>

namespace
{
  namespace option
  {
    constexpr char const* const S {"S"};
    constexpr char const* const K {"K"};
    constexpr char const* const H {"H"};
    constexpr char const* const r {"r"};
    constexpr char const* const sigma {"sigma"};
    constexpr char const* const T {"T"};
    constexpr char const* const timesteps {"timesteps"};
    constexpr char const* const option_type {"option-type"};
    constexpr char const* const barrier_type {"barrier-type"};
  }

  ::boost::program_options::options_description options()
  {
    ::boost::program_options::options_description description;

    //! \todo validators
    description.add_options()
      ( option::S
      , ::boost::program_options::value<double>()->required()
      , "underlying"
      )
      ( option::K
      , ::boost::program_options::value<double>()->required()
      , "strike price"
      )
      ( option::H
      , ::boost::program_options::value<double>()->required()
      , "Barrier"
      )
      ( option::r
      , ::boost::program_options::value<double>()->required()
      , "interest rate"
      )
      ( option::sigma
      , ::boost::program_options::value<double>()->required()
      , "volatility"
      )
      ( option::T
      , ::boost::program_options::value<double>()->required()
      , "timeframe"
      )
      ( option::timesteps
      , ::boost::program_options::value<int>()->required()
      , "discretization"
      )
      //! \todo validator
      ( option::option_type
      , ::boost::program_options::value<std::string>()->required()
      , "Option type: Call, Put"
      )
      //! \todo validator
      ( option::barrier_type
      , ::boost::program_options::value<std::string>()->required()
      , "Barrier type: Down-and-Out (DaO), Down-and-In (DoI), Up-and-Out (UaO), Up-and-In (UaI)"
      )
      ;

    return description;
  }

  barrieropt::BarrierType read_barrier_type (std::string const& b)
  {
    return b == "DaO" ? barrieropt::DaO
      : b == "DaI" ? barrieropt::DaI
      : b == "UaO" ? barrieropt::UaO
      : b == "UaI" ? barrieropt::UaI
      : throw ::boost::program_options::invalid_option_value
        (( ::boost::format ("invalid barrier '%1%, allowed: DaO, DaI, UaO, UaI")
         % b
         ).str()
        )
      ;
  }

  barrieropt::OptionType read_option_type (std::string const& o)
  {
    return o == "Call" ? barrieropt::Call
      : o == "Put" ? barrieropt::Put
      : throw ::boost::program_options::invalid_option_value
        ((::boost::format ("invalid option '%1%', allowed: Call, Put") % o).str())
      ;
  }
}

int main (int argc, char** argv)
try
{
  stochastic_with_heureka::workflow_result const workflow_result
    ( stochastic_with_heureka::run
      ( argc
      , argv
      , options()
      , "barropt"
      , [] (::boost::program_options::variables_map const& vm)
        {
          barrieropt::SingleBarrierOptionMonteCarlo_user_data const implementation_parameter
            { vm[option::S].as<double>()
            , vm[option::K].as<double>()
            , vm[option::H].as<double>()
            , vm[option::r].as<double>()
            , vm[option::sigma].as<double>()
            , vm[option::T].as<double>()
            , vm[option::timesteps].as<int>()
            , read_option_type (vm[option::option_type].as<std::string>())
            , read_barrier_type (vm[option::barrier_type].as<std::string>())
            };

          //! \todo use validators, as noted above
          if (implementation_parameter.S <= 0)
          {
            throw ::boost::program_options::invalid_option_value
              ("requires S > 0");
          }
          if (implementation_parameter.K <= 0)
          {
            throw ::boost::program_options::invalid_option_value
              ("requires K > 0");
          }
          if (implementation_parameter.H <= 0)
          {
            throw ::boost::program_options::invalid_option_value
              ("requires H > 0");
          }
          if (implementation_parameter.r < 0)
          {
            throw ::boost::program_options::invalid_option_value
              ("requires r >= 0");
          }
          if (implementation_parameter.sigma <= 0)
          {
            throw ::boost::program_options::invalid_option_value
              ("requires sigma > 0");
          }
          if (implementation_parameter.T <= 0)
          {
            throw ::boost::program_options::invalid_option_value
              ("requires T > 0");
          }
          if (implementation_parameter.TimeSteps < 1)
          {
            throw ::boost::program_options::invalid_option_value
              ("requires TimeSteps >= 1");
          }

          return we::type::bytearray (implementation_parameter);
        }
      )
    );

  std::cout << "got_heureka = "
            << std::boolalpha << workflow_result.got_heureka() << std::endl;
  std::cout << "number_of_rolls_done = "
            << workflow_result.number_of_rolls_done() << std::endl;

  std::pair<double, double> result;
  workflow_result.result().copy (&result);

  std::cout << "value = " << result.first << std::endl;
  std::cout << "stddev = " << result.second << std::endl;

  return 0;
}
catch (std::exception const& e)
{
  std::cout << "EXCEPTION: " << util::print_exception (e) << std::endl;

  return -1;
}
