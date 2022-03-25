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

#include <aggregate_sum/parse_parameters_from_commandline.hpp>

#include <boost/program_options.hpp>

#include <cstdlib>
#include <iostream>

namespace aggregate_sum
{
  Parameters parse_parameters_from_commandline
    (ParametersDescription const& driver_opts,
     ParametersDescription const& workflow_opts,
     int argc,
     char** argv
    )
  {
    namespace po = boost::program_options;

    ParametersDescription options;
    options.add_options()("help", "this message");
    options.add (driver_opts);
    options.add (workflow_opts);

    Parameters parameters;
    po::store ( po::command_line_parser (argc, argv)
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
