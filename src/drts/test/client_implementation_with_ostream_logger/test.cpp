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

#include <drts/private/option.hpp>

#include <logging/stream_receiver.hpp>

#include <test/certificates_data.hpp>
#include <test/make.hpp>
#include <test/parse_command_line.hpp>
#include <test/scoped_nodefile_from_environment.hpp>
#include <test/shared_directory.hpp>
#include <test/source_directory.hpp>

#include <util-generic/hostname.hpp>
#include <util-generic/join.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/list.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/random.hpp>

#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/boost/program_options/generic.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <list>

BOOST_DATA_TEST_CASE
  (client_implementation_with_ostream_logger, certificates_data, certificates)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::scoped_rifd());
  //! \todo switch to generic interface
  options_description.add_options()
    ( "implementation"
    , boost::program_options::value
      <fhg::util::boost::program_options::existing_path>()->required()
    , "implementation"
    );

  boost::program_options::variables_map vm
    ( test::parse_command_line
        ( boost::unit_test::framework::master_test_suite().argc
        , boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    ( test::shared_directory (vm)
    / "client_implementation_with_ostream_logger"
    );

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / boost::filesystem::unique_path());
  boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net_lib_install const make
    ( installation
    , "client_implementation_with_ostream_logger"
    , test::source_directory (vm)
    , installation_dir
    , test::option::options()
    . add<test::option::gen::include>
        //! \todo urgh
        (test::source_directory (vm).parent_path().parent_path().parent_path())
    );

  gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                 , gspc::rifd::hostnames {vm}
                                 , gspc::rifd::port {vm}
                                 , installation
                                 );

  gspc::scoped_runtime_system drts
    (vm, installation, "worker:1", rifds.entry_points(), std::cerr, certificates);

  std::list<std::string> logged;
  fhg::logging::stream_receiver const log_receiver
    ( drts.top_level_log_demultiplexer()
    , [&logged] (fhg::logging::message const& message)
      {
        //! \note agent/worker do emit messages we do not want.
        if (message._category == fhg::logging::legacy::category_level_info)
        {
          logged.emplace_back (message._content);
        }
      }
    );

  boost::filesystem::path const implementation
    ( vm.at ("implementation")
    . as<fhg::util::boost::program_options::existing_path>()
    );

  std::list<std::string> lines;

  for (int i (0); i < 10; ++i)
  {
    using random_string = fhg::util::testing::random<std::string>;
    // - \n because that's what we join/split on
    // - \\ and \" because the string value parser breaks
    lines.emplace_back (random_string{} (random_string::except ("\n\\\"")));
  }

  // - "flush" all lines with an "\n", because otherwise an empty last
  //   it is "swallowed" (top/gpispace#823)
  auto const message (fhg::util::join (lines, '\n').string() + "\n");

  std::multimap<std::string, pnet::type::value::value_type> const result
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
