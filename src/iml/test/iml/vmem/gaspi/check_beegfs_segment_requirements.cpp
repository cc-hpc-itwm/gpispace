// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
