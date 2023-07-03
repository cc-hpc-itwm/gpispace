// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <bin/run.hpp>
#include <util/print_exception.hpp>

#include <exception>
#include <ios>
#include <iostream>
#include <ostream>

namespace
{
  namespace option
  {
    constexpr char const* const mean {"mean"};
    constexpr char const* const stddev {"stddev"};
  }

  ::boost::program_options::options_description options()
  {
    ::boost::program_options::options_description description;

    description.add_options()
      ( option::mean
      , ::boost::program_options::value<unsigned int>()->required()
      , "mean duration in milliseconds"
      );
    description.add_options()
      ( option::stddev
      , ::boost::program_options::value<unsigned int>()->required()
      , "standard deviation if task duration in milliseconds"
      );

    return description;
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
      , "tasks_with_normal_distributed_duration"
      , [] (::boost::program_options::variables_map const& vm)
        {
          return we::type::bytearray
            ( std::make_pair
              ( vm[option::mean].as<unsigned int>()
              , vm[option::stddev].as<unsigned int>()
              )
            );
        }
      )
    );

  std::cout << "got_heureka = "
            << std::boolalpha << workflow_result.got_heureka() << std::endl;
  std::cout << "number_of_rolls_done = "
            << workflow_result.number_of_rolls_done() << std::endl;

  unsigned long number_of_tasks;
  workflow_result.result().copy (&number_of_tasks);

  std::cout << "number_of_tasks = " << number_of_tasks << std::endl;

  return 0;
}
catch (std::exception const& e)
{
  std::cout << "EXCEPTION: " << util::print_exception (e) << std::endl;

  return -1;
}
