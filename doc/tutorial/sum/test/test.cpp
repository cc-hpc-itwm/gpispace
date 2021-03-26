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

#include <boost/test/unit_test.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/source_directory.hpp>
#include <test/shared_directory.hpp>

#include <we/type/value.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/system_with_blocked_SIGCHLD.hpp>
#include <util-generic/nest_exceptions.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/range/adaptor/map.hpp>

#include <algorithm>
#include <map>
#include <vector>

namespace
{
  void run_and_check
    ( boost::program_options::variables_map const& vm
    , gspc::installation const& installation
    , boost::filesystem::path const& pnet
    )
  {
    gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                   , gspc::rifd::hostnames {vm}
                                   , gspc::rifd::port {vm}
                                   , installation
                                   );
    gspc::scoped_runtime_system const drts
      (vm, installation, "work:4", rifds.entry_points());

    auto pair
      ( [] (long x, long y) -> pnet::type::value::value_type
      {
        pnet::type::value::value_type v;
        pnet::type::value::poke ("x", v, x);
        pnet::type::value::poke ("y", v, y);
        return v;
      }
      );

    std::multimap<std::string, pnet::type::value::value_type> const result
      ( gspc::client (drts)
      . put_and_run ( gspc::workflow (pnet)
                    , {{"p", pair (3, 4)}, {"p", pair (-2, 3)}}
                    )
      );

    decltype (result) const expected {{"s", 1L}, {"s", 7L}};
    FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (result, expected);
  }
}

BOOST_AUTO_TEST_CASE (tutorial_sum_expr)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());

  boost::program_options::variables_map vm
    ( test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "tutorial_sum_expr");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net const make
    ( installation
    , "sum_expr_many"
    , test::source_directory (vm)
    );

  run_and_check (vm, installation, make.pnet());
}

BOOST_AUTO_TEST_CASE (tutorial_sum_mod)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());

  boost::program_options::variables_map vm
    ( test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    (test::shared_directory (vm) / "tutorial_sum_mod");

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  fhg::util::temporary_path const _sum_module_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const sum_module_dir (_sum_module_dir);

  fhg::util::nest_exceptions<std::runtime_error>
    ([&sum_module_dir, &vm]()
     {
       std::ostringstream make_module;

       make_module
         << "make"
         << " DIR_BUILD=" << sum_module_dir
         << " -C " << (test::source_directory (vm) / "src")
         ;

       fhg::util::system_with_blocked_SIGCHLD (make_module.str());
     }
    , "Could not 'make sum_module'"
    );

  gspc::installation const installation (vm);

  test::make_net_lib_install const make
    ( installation
    , "sum_many"
    , test::source_directory (vm)
    , installation_dir
    , test::option::options()
    . add<test::option::gen::link> (sum_module_dir / "sum.o")
    . add<test::option::gen::include> (test::source_directory (vm) / "include")
    );

  run_and_check (vm, installation, make.pnet());
}
