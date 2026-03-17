// Copyright (C) 2014-2016,2021-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/drts/client.hpp>
#include <gspc/drts/drts.hpp>
#include <gspc/drts/scoped_rifd.hpp>

#include <gspc/testing/make.hpp>
#include <gspc/testing/parse_command_line.hpp>
#include <gspc/testing/scoped_nodefile_from_environment.hpp>
#include <gspc/testing/shared_directory.hpp>
#include <gspc/testing/source_directory.hpp>

#include <gspc/we/type/value.hpp>

#include <gspc/util/system_with_blocked_SIGCHLD.hpp>
#include <gspc/util/boost/program_options/validators/executable.hpp>
#include <gspc/util/temporary_file.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/unique_path.hpp>

#include <boost/program_options.hpp>

#include <exception>
#include <filesystem>
#include <map>
#include <sstream>
#include <stdexcept>

BOOST_AUTO_TEST_CASE (doc_tutorial_avg_stddev)
{
  namespace validators = gspc::util::boost::program_options;

  ::boost::program_options::options_description options_description;

  constexpr char const* const option_name_generator ("generator");

  options_description.add_options()
    ( option_name_generator
    , ::boost::program_options::value<validators::executable>()->required()
    , "generator program"
    )
    ;

  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());

  ::boost::program_options::variables_map vm
    ( gspc::testing::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  gspc::util::temporary_path const shared_directory
    (gspc::testing::shared_directory (vm) / "doc_tutorial_avg_stddev");

  gspc::testing::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  gspc::util::temporary_path const _installation_dir
    ( std::filesystem::path {shared_directory}
    / gspc::testing::unique_path()
    );
  auto const installation_dir {std::filesystem::path {_installation_dir}};

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  gspc::util::temporary_file _data_file
    ( std::filesystem::path {shared_directory}
    / gspc::testing::unique_path()
    );
  std::filesystem::path const data_file (_data_file);

  gspc::testing::make_net_lib_install const make
    ( installation
    , "avg_stddev"
    , gspc::testing::source_directory (vm)
    , installation_dir
    , gspc::testing::option::options()
    . add<gspc::testing::option::gen::include> (gspc::testing::source_directory (vm) / "include")
    . add<gspc::testing::option::gen::cxx_flag> ("--std=c++17")
    );

  auto const generator
    ( std::filesystem::canonical
      (vm.at (option_name_generator).as<validators::executable>())
    );

  long const num_values (100 << 20);
  long const size_chunk (8 << 20);
  long const size_buffer (8 << 20);
  long const num_buffer (10);

  try
  {
    //! \todo inline the generator code instead of calling a binary
    std::ostringstream command_generate;

    command_generate
      << generator
      << " -b " << size_buffer
      << " -n " << num_values
      << " -s 31415926"
      << " -m 0"
      << " -g 1"
      << " -o " << data_file
      ;

    gspc::util::system_with_blocked_SIGCHLD (command_generate.str());
  }
  catch (...)
  {
    std::throw_with_nested (std::runtime_error {"Could not generate data"});
  }

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );
  gspc::scoped_runtime_system const drts
    (vm, installation, "worker:4", rifds.entry_points());

  std::multimap<std::string, gspc::pnet::type::value::value_type> const result
    ( gspc::client (drts)
    . put_and_run ( gspc::workflow (make.pnet())
                  , { {"name_file", data_file.string()}
                    , {"size_file", long (sizeof (long) * num_values)}
                    , {"size_chunk", size_chunk}
                    , {"size_buffer", size_buffer}
                    , {"num_buffer", num_buffer}
                    }
                  )
    );

  BOOST_REQUIRE_EQUAL (result.size(), 2);

  std::string const port_avg ("avg");
  std::string const port_stddev ("stddev");

  BOOST_REQUIRE_EQUAL (result.count (port_avg), 1);
  BOOST_REQUIRE_EQUAL (result.count (port_stddev), 1);

  BOOST_CHECK_SMALL
    (::boost::get<double> (result.find (port_avg)->second), 1e-3);
  BOOST_CHECK_CLOSE_FRACTION
    (::boost::get<double> (result.find (port_stddev)->second), 1.0, 1e-3);
}
