// Copyright (C) 2015,2019,2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/rif/started_process_promise.hpp>

#include <gspc/util/boost/program_options/generic.hpp>

#include <gspc/util/hostname.hpp>

namespace
{
  namespace option
  {
    namespace po = gspc::util::boost::program_options;

    po::option<std::string> const option {"option", "option"};
  }
}

int main (int ac, char **av)
{
  gspc::rif::started_process_promise promise (ac, av);

  try
  {
    ::boost::program_options::variables_map const vm
      ( gspc::util::boost::program_options::options ("test_binary")
      . require (option::option)
      . store_and_notify (ac, av)
      );

    promise.set_result ( option::option.get_from (vm)
                       , gspc::util::hostname()
                       );

    return 0;
  }
  catch (...)
  {
    promise.set_exception (std::current_exception());

    return EXIT_FAILURE;
  }
}
