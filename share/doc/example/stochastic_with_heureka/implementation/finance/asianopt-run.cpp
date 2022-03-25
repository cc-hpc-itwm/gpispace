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

#include <implementation/finance/asianopt.hpp>

#include <bin/run.hpp>
#include <util/print_exception.hpp>

#include <boost/format.hpp>

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
  namespace option
  {
    constexpr char const* const S {"S"};
    constexpr char const* const K {"K"};
    constexpr char const* const T {"T"};
    constexpr char const* const sigma {"sigma"};
    constexpr char const* const r {"r"};
    constexpr char const* const d {"d"};
    constexpr char const* const first_fixing {"first-fixing"};
    constexpr char const* const fixings_per_year {"fixings-per-year"};
    constexpr char const* const option_type {"option-type"};
    constexpr char const* const control_variate {"control-variate"};
  }

  ::boost::program_options::options_description options()
  {
    ::boost::program_options::options_description description;

    //! \todo validators
    description.add_options()
      ( option::S
      , ::boost::program_options::value<double>()->required()
      , "stock price"
      )
      ( option::K
      , ::boost::program_options::value<double>()->required()
      , "Strike"
      )
      ( option::T
      , ::boost::program_options::value<double>()->required()
      , "maturity"
      )
      ( option::sigma
      , ::boost::program_options::value<double>()->required()
      , "volatility"
      )
      ( option::r
      , ::boost::program_options::value<double>()->required()
      , "interest rate"
      )
      ( option::d
      , ::boost::program_options::value<double>()->required()
      , "d"
      )
      ( option::first_fixing
      , ::boost::program_options::value<int>()->required()
      , "First fixing"
      )
      ( option::fixings_per_year
      , ::boost::program_options::value<double>()->required()
      , "Fixings per year"
      )
      //! \todo validator
      ( option::option_type
      , ::boost::program_options::value<std::string>()->required()
      , "option type: FloC, FloP, FixC, FixP"
      )
      ( option::control_variate
      , ::boost::program_options::value<bool>()->required()
      , "use control variate"
      );

    return description;
  }

  asianopt::OptionType read_option_type (std::string const& o)
  {
    return ( o == "FloC" ? asianopt::FloC
           : o == "FloP" ? asianopt::FloP
           : o == "FixC" ? asianopt::FixC
           : o == "FixP" ? asianopt::FixP
           : throw ::boost::program_options::invalid_option_value
             ( ( ::boost::format
                 ("invalid option '%1%', allowed: FloC, FloP, FixC, FixP")
               % o
               ).str()
             )
           );
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
      , "asianopt"
      , [] (::boost::program_options::variables_map const& vm)
        {
          //! \todo use validators instead of throwing ctor
          try
          {
            return we::type::bytearray
              ( asianopt::Parameters
                ( vm[option::S].as<double>()
                , vm[option::K].as<double>()
                , vm[option::T].as<double>()
                , vm[option::sigma].as<double>()
                , vm[option::r].as<double>()
                , vm[option::d].as<double>()
                , vm[option::first_fixing].as<int>()
                , vm[option::fixings_per_year].as<double>()
                , read_option_type (vm[option::option_type].as<std::string>())
                , vm[option::control_variate].as<bool>()
                )
              );
          }
          catch (std::invalid_argument const& error)
          {
            throw ::boost::program_options::invalid_option_value (error.what());
          }
        }
      )
    );

  std::cout << "got_heureka = "
            << std::boolalpha << workflow_result.got_heureka() << std::endl;
  std::cout << "number_of_rolls_done = "
            << workflow_result.number_of_rolls_done() << std::endl;

  asianopt::Result result;
  workflow_result.result().copy (&result);

  std::cout << "price = " << result.price << std::endl;
  std::cout << "std_dev = " << result.std_dev << std::endl;

  return 0;
}
catch (std::exception const& e)
{
  std::cout << "EXCEPTION: " << util::print_exception (e) << std::endl;

  return -1;
}
