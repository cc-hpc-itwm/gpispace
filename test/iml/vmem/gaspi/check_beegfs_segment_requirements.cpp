// Copyright (C) 2015,2020,2022-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/iml/vmem/segment/beegfs.hpp>

#include <gspc/util/print_exception.hpp>

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

  gspc::iml::vmem::segment::beegfs::check_requirements (argv[1]);

  return 0;
}
catch (...)
{
  std::cerr << gspc::util::current_exception_printer() << "\n";
  return 1;
}
