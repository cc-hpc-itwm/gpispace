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

#include <iml/vmem/segment/beegfs.hpp>

#include <util-generic/print_exception.hpp>

#include <boost/program_options.hpp>

#include <iostream>
#include <stdexcept>

int main (int argc, char** argv)
try
{
  if (argc != 2)
  {
    throw std::invalid_argument ("no directory given");
  }

  fhg::iml::vmem::segment::beegfs::check_requirements (argv[1]);

  return 0;
}
catch (...)
{
  std::cerr << fhg::util::current_exception_printer() << "\n";
  return 1;
}
