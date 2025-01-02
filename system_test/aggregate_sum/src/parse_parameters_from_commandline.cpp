// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <aggregate_sum/parse_parameters_from_commandline.hpp>

#include <boost/program_options.hpp>

#include <cstdlib>
#include <iostream>

namespace aggregate_sum
{
  Parameters parse_parameters_from_commandline
    ( ParametersDescription const& driver_opts
    , ParametersDescription const& workflow_opts
    , int argc
    , char** argv
    )
  {
    ParametersDescription options;
    options.add_options() ("help", "this message");
    options.add (driver_opts);
    options.add (workflow_opts);

    Parameters parameters;
    ::boost::program_options::store
        ( ::boost::program_options::command_line_parser (argc, argv)
          . options (options)
          . run()
        , parameters
        );

    if (parameters.count ("help"))
    {
      std::cout << options << std::endl;
      std::exit (EXIT_SUCCESS);
    }

    parameters.notify();

    return parameters;
  }
}
