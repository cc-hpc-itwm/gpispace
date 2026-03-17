// Copyright (C) 2021,2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <aggregate_sum/execute.hpp>
#include <aggregate_sum/Workflow.hpp>

#include <gspc/drts/client.hpp>
#include <gspc/drts/drts.hpp>
#include <gspc/drts/scoped_rifd.hpp>

#include <gspc/util/executable_path.hpp>

#include <filesystem>
#include <string>

namespace aggregate_sum
{
  namespace execution
  {
    ParametersDescription options()
    {
      ParametersDescription driver_opts {"Worker Topology"};
      driver_opts.add_options()
        ( "topology"
        , ::boost::program_options::value<std::string>()->required()
        );
      driver_opts.add (gspc::options::installation());
      driver_opts.add (gspc::options::drts());
      driver_opts.add (gspc::options::logging());
      driver_opts.add (gspc::options::scoped_rifd());

      return driver_opts;
    }
  }

  WorkflowResult execute (Parameters parameters, Workflow const& workflow)
  {
    std::filesystem::path const aggregate_sum_installation_path
      {gspc::util::executable_path().parent_path().parent_path().string()};

    gspc::installation installation {parameters};
    gspc::scoped_rifds rifds
      { gspc::rifd::strategy {parameters}
      , gspc::rifd::hostnames {parameters}
      , gspc::rifd::port {parameters}
      , installation
      };

    gspc::set_application_search_path
      (parameters, aggregate_sum_installation_path / "lib");

    gspc::scoped_runtime_system drts
      { parameters
      , installation
      , parameters.at ("topology").as<std::string>()
      , rifds.entry_points()
      };

    return gspc::client {drts}
      . put_and_run
        ( aggregate_sum_installation_path / "pnet" / "aggregate_sum.pnet"
        , workflow.inputs().map()
        );
  }
}
