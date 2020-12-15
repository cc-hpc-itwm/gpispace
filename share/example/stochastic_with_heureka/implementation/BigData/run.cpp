// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <bin/run.hpp>

#include <implementation/BigData/barropt.h>

#include <boost/format.hpp>

#include <iostream>
#include <iomanip>

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
    constexpr char const* const option_typ {"option-typ"};
    constexpr char const* const barrier_typ {"barrier-typ"};
  }

  boost::program_options::options_description options()
  {
    boost::program_options::options_description description;

    //! \todo validators
    description.add_options()
      ( option::S
      , boost::program_options::value<double>()->required()
      , "Basiswert"
      )
      ( option::K
      , boost::program_options::value<double>()->required()
      , "Ausübungspreis"
      )
      ( option::H
      , boost::program_options::value<double>()->required()
      , "Barrier"
      )
      ( option::r
      , boost::program_options::value<double>()->required()
      , "Zinsrate"
      )
      ( option::sigma
      , boost::program_options::value<double>()->required()
      , "Volatilität"
      )
      ( option::T
      , boost::program_options::value<double>()->required()
      , "Laufzeit"
      )
      ( option::timesteps
      , boost::program_options::value<int>()->required()
      , "Diskretisierung"
      )
      //! \todo validator
      ( option::option_typ
      , boost::program_options::value<std::string>()->required()
      , "Optionsart: Call, Put"
      )
      //! \todo validator
      ( option::barrier_typ
      , boost::program_options::value<std::string>()->required()
      , "Barrieretyp: Down-and-Out (DaO), Down-and-In (DoI), Up-and-Out (UaO), Up-and-In (UaI)"
      )
      ;

    return description;
  }

  BarrierTyp read_barrier_typ (std::string const& b)
  {
    return b == "DaO" ? DaO
      : b == "DaI" ? DaI
      : b == "UaO" ? UaO
      : b == "UaI" ? UaI
      : throw boost::program_options::invalid_option_value
        (( boost::format ("invalid barrier '%1%, allowed: DaO, DaI, UaO, UaI")
         % b
         ).str()
        )
      ;
  }

  OptionTyp read_option_typ (std::string const& o)
  {
    return o == "Call" ? Call
      : o == "Put" ? Put
      : throw boost::program_options::invalid_option_value
        ((boost::format ("invalid option '%1%', allowed: Call, Put") % o).str())
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
      , [] (boost::program_options::variables_map const& vm)
        {
          SingleBarrierOptionMonteCarlo_user_data const implementation_parameter
            { vm[option::S].as<double>()
            , vm[option::K].as<double>()
            , vm[option::H].as<double>()
            , vm[option::r].as<double>()
            , vm[option::sigma].as<double>()
            , vm[option::T].as<double>()
            , vm[option::timesteps].as<int>()
            , read_option_typ (vm[option::option_typ].as<std::string>())
            , read_barrier_typ (vm[option::barrier_typ].as<std::string>())
            };

          //! \todo use validators, as noted above
          if (implementation_parameter.S <= 0)
          {
            throw boost::program_options::invalid_option_value
              ("requires S > 0");
          }
          if (implementation_parameter.K <= 0)
          {
            throw boost::program_options::invalid_option_value
              ("requires K > 0");
          }
          if (implementation_parameter.H <= 0)
          {
            throw boost::program_options::invalid_option_value
              ("requires H > 0");
          }
          if (implementation_parameter.r < 0)
          {
            throw boost::program_options::invalid_option_value
              ("requires r >= 0");
          }
          if (implementation_parameter.sigma <= 0)
          {
            throw boost::program_options::invalid_option_value
              ("requires sigma > 0");
          }
          if (implementation_parameter.T <= 0)
          {
            throw boost::program_options::invalid_option_value
              ("requires T > 0");
          }
          if (implementation_parameter.TimeSteps < 1)
          {
            throw boost::program_options::invalid_option_value
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
  std::cout << "EXCEPTION: " << e.what() << std::endl;

  return -1;
}
