// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <util.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/private/option.hpp>
#include <drts/scoped_rifd.hpp>

#include <testing/hopefully_free_port.hpp>
#include <testing/make.hpp>
#include <testing/parse_command_line.hpp>
#include <testing/scoped_nodefile_from_environment.hpp>
#include <testing/shared_directory.hpp>
#include <testing/source_directory.hpp>

#include <util-generic/finally.hpp>
#include <util-generic/read_lines.hpp>
#include <util-generic/temporary_file.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/require_container_is_permutation.hpp>

#include <we/type/value/boost/test/printer.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <vector>

namespace
{
  // \note If this test failed, double check if the exceptions that
  // are thrown are still the same, this heuristic isn't perfect.
  bool should_retry (std::runtime_error const& exception)
  {
    try
    {
      std::rethrow_if_nested (exception);
    }
    catch (std::runtime_error const& nested)
    {
      return should_retry (nested);
    }

    return std::string (exception.what()).find ("bind: Address already in use")
      != std::string::npos;
  }
}

BOOST_AUTO_TEST_CASE (use_fixed_ports_for_agents_and_workers)
{
  ::boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (test::options::shared_directory());
  options_description.add (gspc::options::installation());
  options_description.add (gspc::options::drts());
  options_description.add (gspc::options::logging());
  options_description.add (gspc::options::scoped_rifd());

  ::boost::program_options::variables_map vm
    ( test::parse_command_line
        ( ::boost::unit_test::framework::master_test_suite().argc
        , ::boost::unit_test::framework::master_test_suite().argv
        , options_description
        )
    );

  fhg::util::temporary_path const shared_directory
    ( test::shared_directory (vm)
    / "use_fixed_ports_for_agents_and_workers"
    );

  test::scoped_nodefile_from_environment const nodefile_from_environment
    (shared_directory, vm);

  fhg::util::temporary_path const _installation_dir
    (shared_directory / ::boost::filesystem::unique_path());
  ::boost::filesystem::path const installation_dir (_installation_dir);

  gspc::set_application_search_path (vm, installation_dir);

  auto const vm_without_ports (vm);

  vm.notify();

  gspc::installation const installation (vm);

  test::make_net_lib_install const make
    ( installation
    , "use_fixed_ports_for_agents_and_workers"
    , test::source_directory (vm)
    , installation_dir
    , test::option::options()
    . add<test::option::gen::include> (test::source_directory (vm))
    . add<test::option::gen::include>
        (test::source_directory (vm).parent_path().parent_path().parent_path())
    );

  std::vector<std::string> const hosts
    (fhg::util::read_lines (nodefile_from_environment.path()));

  BOOST_REQUIRE_GT (hosts.size(), 0);

  gspc::scoped_rifds rifds
    ( gspc::rifd::strategy (vm)
    , gspc::rifd::hostnames ({hosts.front()})
    , gspc::rifd::port (vm)
    , installation
    );

  std::unique_ptr<gspc::scoped_runtime_system> drts;
  std::unique_ptr<test::hopefully_free_port> agent_port;
  std::unique_ptr<test::hopefully_free_port> worker_port;

  // Even if we directly allocate a port, release it and let the DRTS
  // use it, there still is a race that the port could be already in
  // use by a different process again. This is quite rare but for the
  // sake of tests not spuriously failing in this case, give this part
  // of the test more than one try as it is not what is wanted to
  // test: We are interested if after startup *we* use those ports,
  // not if we fail starting up because someone else is using them
  // (which implicitly also confirms test success but just for one
  // component).
  int tries (0);
  while (!drts)
  {
    try
    {
      agent_port = std::make_unique<test::hopefully_free_port>();
      worker_port = std::make_unique<test::hopefully_free_port>();

      auto vm_with_ports (vm_without_ports);
      gspc::set_agent_port (vm_with_ports, agent_port->release());
      vm_with_ports.notify();

      drts = std::make_unique<gspc::scoped_runtime_system>
               ( vm_with_ports
               , installation
               , "worker:1/" + std::to_string (worker_port->release())
               , rifds.entry_points()
               );
    }
    catch (std::runtime_error const& err)
    {
      if (tries++ > 5 || !should_retry (err))
      {
        throw;
      }
    }
  }

  BOOST_REQUIRE (is_using_port ("agent", *agent_port));

  gspc::client client (*drts);

  std::multimap<std::string, pnet::type::value::value_type> const result
    ( client.put_and_run
        ( gspc::workflow (make.pnet())
        , { {"port", static_cast<unsigned int> (*worker_port)}
          , {"start", true}
          }
        )
    );

  decltype (result) const expected {{"port_is_used", true}};
  FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (expected, result);
}
