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

#pragma once

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>
#include <drts/stream.hpp>
#include <drts/virtual_memory.hpp>

#include <testing/make.hpp>
#include <testing/parse_command_line.hpp>
#include <testing/scoped_nodefile_from_environment.hpp>
#include <testing/shared_directory.hpp>
#include <testing/source_directory.hpp>
#include <testing/virtual_memory_socket_name_for_localhost.hpp>

#include <we/type/value.hpp>
#include <we/type/value/peek.hpp>

#include <util-generic/boost/program_options/validators/positive_integral.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/set.hpp>
#include <util-generic/read_file.hpp>
#include <util-generic/temporary_path.hpp>

#include <util-generic/ndebug.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/range/adaptor/map.hpp>

#include <chrono>
#include <functional>
#include <set>
#include <string>
#include <thread>

namespace share_example_stream_test
{
  template<typename R, typename P>
  void run
    ( std::string const& workflow_name
    , std::function<std::string (unsigned long size_slot)> const& topology
    , std::chrono::duration<R, P> const& sleep_after_produce
    , double IFDEF_NDEBUG (allowed_average_round_trip_time)
    )
  {
    namespace validators = fhg::util::boost::program_options;

    ::boost::program_options::options_description options_description;

    constexpr char const* const option_num_slots ("num-slots");
    constexpr char const* const option_size_slot ("size-slot");

    options_description.add_options()
      ( option_num_slots
      , ::boost::program_options::value
      <validators::positive_integral<unsigned long>>()->required()
      , "number of slots in virtual memory"
      )
      ( option_size_slot
      , ::boost::program_options::value
      <validators::positive_integral<unsigned long>>()->required()
      , "size of slots in virtual memory"
      )
      ;

    options_description.add (test::options::shared_directory());
    options_description.add (test::options::source_directory());
    options_description.add (gspc::options::installation());
    options_description.add (gspc::options::drts());
    options_description.add (gspc::options::scoped_rifd());
    options_description.add (gspc::options::virtual_memory());

    ::boost::program_options::variables_map vm
      ( test::parse_command_line
          ( ::boost::unit_test::framework::master_test_suite().argc
          , ::boost::unit_test::framework::master_test_suite().argv
          , options_description
          )
      );

    fhg::util::temporary_path const shared_directory
      (test::shared_directory (vm) / ("share_example_stream" + workflow_name));

    test::scoped_nodefile_from_environment const nodefile_from_environment
      (shared_directory, vm);

    fhg::util::temporary_path const _installation_dir
      (shared_directory / ::boost::filesystem::unique_path());
    ::boost::filesystem::path const installation_dir (_installation_dir);

    gspc::set_application_search_path (vm, installation_dir);
    test::set_virtual_memory_socket_name_for_localhost (vm);

    vm.notify();

    gspc::installation const installation (vm);

    test::make_net_lib_install const make
      ( installation
      , workflow_name
      , test::source_directory (vm)
      , installation_dir
      , test::option::options()
      . add<test::option::gen::include> (test::source_directory (vm))
      );

    auto const num_slots
      (vm.at (option_num_slots).as<validators::positive_integral<unsigned long>>());
    auto const size_slot
      (vm.at (option_size_slot).as<validators::positive_integral<unsigned long>>());

    unsigned long const size (num_slots * (size_slot + 1));

    gspc::scoped_rifds const rifds ( gspc::rifd::strategy {vm}
                                   , gspc::rifd::hostnames {vm}
                                   , gspc::rifd::port {vm}
                                   , installation
                                   );
    gspc::scoped_runtime_system const drts
      (vm, installation, topology (size_slot), rifds.entry_points());

    gspc::vmem_allocation const allocation_buffer
      ( drts.alloc ( gspc::vmem::gaspi_segment_description()
                   , size
                   , workflow_name + "_buffer"
                   )
      );
    gspc::client client (drts);

    gspc::workflow workflow (make.pnet());

    workflow.set_wait_for_output();

    unsigned long const rounds (20 * num_slots);

    gspc::job_id_t const job_id
      (client.submit (workflow, {{"rounds", rounds}}));

    gspc::stream stream
      (drts.create_stream ( "stream_test"
                          , allocation_buffer
                          , size_slot
                          , [&client, &job_id] (pnet::type::value::value_type const& value) -> void
                            {
                              client.put_token (job_id, "work_package", value);
                            }
                          )
      );

    std::chrono::high_resolution_clock clock;

    std::set<std::string> expected_output;

    for (unsigned long id (0); id < rounds; ++id)
    {
      std::chrono::high_resolution_clock::rep const now
        ( std::chrono::duration_cast<std::chrono::microseconds>
          (clock.now().time_since_epoch()).count()
        );

      std::string const data ((::boost::format ("%1% %2%") % id % now).str());

      expected_output.emplace (data);

      stream.write (data);

      std::this_thread::sleep_for (sleep_after_produce);
    }

    client.put_token (job_id, "stop", we::type::literal::control());

    std::multimap<std::string, pnet::type::value::value_type> const result
      (client.wait_and_extract (job_id));

    BOOST_REQUIRE_EQUAL (result.count ("done"), 1);
    BOOST_REQUIRE_EQUAL (result.count ("statistic"), 1);
    BOOST_REQUIRE_EQUAL (result.count ("packages"), rounds);

#ifdef NDEBUG
    {
      pnet::type::value::value_type const& statistic
        (result.find ("statistic")->second);

      BOOST_REQUIRE (!!pnet::type::value::peek ("count", statistic));
      BOOST_REQUIRE_EQUAL
        ( ::boost::get<unsigned long>
          (*pnet::type::value::peek ("count", statistic)) + 1UL
        , rounds
        );

      BOOST_REQUIRE (!!pnet::type::value::peek ("sum", statistic));
      BOOST_REQUIRE_LE
        ( ::boost::get<double> (*pnet::type::value::peek ("sum", statistic))
        , rounds * allowed_average_round_trip_time
        );
    }
#endif

    std::set<std::string> output;

    for ( pnet::type::value::value_type const& package
        : result.equal_range ("packages") | ::boost::adaptors::map_values
        )
    {
      output.emplace (::boost::get<std::string> (package));
    }

    BOOST_REQUIRE_EQUAL (expected_output, output);
  }
}
