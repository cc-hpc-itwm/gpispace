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

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <boost/program_options.hpp>

#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

int main (int argc, char** argv)
try
{
  boost::program_options::options_description options ("compute_and_aggregate");
  options
    .add (gspc::options::drts())
    .add (gspc::options::logging())
    .add (gspc::options::scoped_rifd())
    .add_options()
       ("N", boost::program_options::value<int>()->required())
       ("workers-per-node", boost::program_options::value<int>()->required())
       ("help", boost::program_options::bool_switch()->default_value (false));

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser (argc, argv)
      .options (options).run()
    , vm
    );

  if (vm.at ("help").as<bool>()) {
    std::cout << "Usage:\n" << options << "\n";
    return 0;
  }

  vm.notify();

  int const workers_per_node (vm.at ("workers-per-node").as<int>());
  int const N (vm.at ("N").as<int>());

  boost::filesystem::path const app_install_dir (APP_INSTALL_DIR);
  gspc::set_application_search_path (vm, app_install_dir / "lib");

  gspc::installation const gspc_installation (GPISPACE_INSTALL_DIR);

  gspc::scoped_rifds const rifds
    ( gspc::rifd::strategy {vm}   // ssh or pbdsh
    , gspc::rifd::hostnames {vm}  // a vector of host names
    , gspc::rifd::port {vm}       // port for communication
    , gspc_installation
    );

  // define topology for GPI-Space: workers_per_node workers with
  // capability/name "worker" per host.
  std::string const topology_description
    ("worker:" + std::to_string (workers_per_node));

  gspc::scoped_runtime_system drts
    ( vm                    // variables_map containing GPI-Space options
    , gspc_installation
    , topology_description
    , rifds.entry_points()  // entry points to start agent and workers on
    );

  gspc::workflow const workflow
    (app_install_dir / "compute_and_aggregate.pnet");

  gspc::client client (drts);

  std::multimap<std::string, pnet::type::value::value_type> values_on_ports;

  // put N values onto place "trigger" via input port "task_trigger"
  // to trigger N "compute" tasks
  for (int i (0); i < N; ++i) {
    values_on_ports.emplace ("task_trigger", we::type::literal::control{});
  }

  auto const results (client.put_and_run (workflow, values_on_ports));

  if (results.size() != 1 || results.count ("aggregated_value") != 1) {
    throw std::logic_error ("unexpected output");
  }

  pnet::type::value::value_type const final_result_value
    (results.find ("aggregated_value")->second);

  std::cout << boost::get<unsigned int> (final_result_value) << std::endl;

  return 0;
}
catch (std::exception const& ex)
{
  std::cerr << "Error: " << ex.what() << "\n";
  return 1;
}
