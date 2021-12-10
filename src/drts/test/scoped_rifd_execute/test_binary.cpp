// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <rif/started_process_promise.hpp>

#include <util-generic/boost/program_options/generic.hpp>

#include <util-generic/hostname.hpp>

namespace
{
  namespace option
  {
    namespace po = fhg::util::boost::program_options;

    po::option<std::string> const option {"option", "option"};
  }
}

int main (int ac, char **av)
{
  fhg::rif::started_process_promise promise (ac, av);

  try
  {
    ::boost::program_options::variables_map const vm
      ( fhg::util::boost::program_options::options ("test_binary")
      . require (option::option)
      . store_and_notify (ac, av)
      );

    promise.set_result ( option::option.get_from (vm)
                       , fhg::util::hostname()
                       );

    return 0;
  }
  catch (...)
  {
    promise.set_exception (std::current_exception());

    return EXIT_FAILURE;
  }
}
