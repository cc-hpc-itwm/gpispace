// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
