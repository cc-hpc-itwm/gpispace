// Copyright (C) 2015-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/drts/client.hpp>
#include <gspc/drts/drts.hpp>
#include <gspc/drts/scoped_rifd.hpp>

#include <gspc/drts/private/option.hpp>

#include <gspc/logging/stream_receiver.hpp>

#include <gspc/testing/certificates_data.hpp>
#include <gspc/testing/make.hpp>
#include <gspc/testing/parse_command_line.hpp>
#include <gspc/testing/scoped_nodefile_from_environment.hpp>
#include <gspc/testing/shared_directory.hpp>
#include <gspc/testing/source_directory.hpp>

#include <gspc/util/hostname.hpp>
#include <gspc/util/join.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/list.hpp>
#include <gspc/testing/printer/optional.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/unique_path.hpp>

#include <gspc/util/boost/program_options/generic.hpp>
#include <gspc/util/boost/program_options/validators/existing_path.hpp>

#include <filesystem>

#include <boost/program_options.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <list>

BOOST_DATA_TEST_CASE
  (client_implementation_with_ostream_logger, certificates_data, certificates)
{
  ::boost::program_options::options_description options_description;

  options_description.add (gspc::testing::options::source_directory());
  options_description.add (gspc::testing::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  //! \todo switch to generic interface
  options_description.add_options()
    ( "implementation"
    , ::boost::program_options::value
      <gspc::util::boost::program_options::existing_path>()->required()
    , "implementation"
    );

  ::boost::program_options::variables_map vm
    ( gspc::testing::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  gspc::util::temporary_path const shared_directory
    ( gspc::testing::shared_directory (vm)
    / "client_implementation_with_ostream_logger"
    );

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

  gspc::testing::make_net_lib_install const make
    ( installation
    , "client_implementation_with_ostream_logger"
    , gspc::testing::source_directory (vm)
    , installation_dir
    , gspc::testing::option::options()
    . add<gspc::testing::option::gen::include>
        //! \todo urgh
        (gspc::testing::source_directory (vm).parent_path().parent_path().parent_path())
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );

  gspc::scoped_runtime_system drts
    (vm, installation, "worker:1", rifds.entry_points(), std::cerr, certificates);

  std::list<std::string> logged;
  gspc::logging::stream_receiver const log_receiver
    ( drts.top_level_log_demultiplexer()
    , [&logged] (gspc::logging::message const& message)
      {
        //! \note agent/worker do emit messages we do not want.
        if (message._category == gspc::logging::legacy::category_level_info)
        {
          logged.emplace_back (message._content);
        }
      }
    );

  std::filesystem::path const implementation
    ( vm.at ("implementation")
    . as<gspc::util::boost::program_options::existing_path>()
    );

  std::list<std::string> lines;

  for (int i (0); i < 10; ++i)
  {
    using random_string = gspc::testing::random<std::string>;
    // - \n because that's what we join/split on
    // - \\ and \" because the string value parser breaks
    lines.emplace_back (random_string{} (random_string::except ("\n\\\"")));
  }

  // - "flush" all lines with an "\n", because otherwise an empty last
  //   it is "swallowed" (top/gpispace#823)
  auto const message (gspc::util::join (lines, '\n').string() + "\n");

  std::multimap<std::string, gspc::pnet::type::value::value_type> const result
    ( gspc::client (drts, certificates).put_and_run
      ( gspc::workflow (make.pnet())
      , { {"implementation", implementation.string()}
        , {"message", message}
        }
      )
    );

  BOOST_REQUIRE_EQUAL (result.size(), 0);
  BOOST_REQUIRE_EQUAL (lines, logged);
}
