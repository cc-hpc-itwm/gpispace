// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <preferences_and_multimodules/parse_parameters_from_commandline.hpp>
#include <preferences_and_multimodules/parameters.hpp>
#include <preferences_and_multimodules/workflow.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>
#include <drts/scoped_rifd.hpp>

#include <util-generic/executable_path.hpp>

#include <fmt/core.h>
#include <string>

namespace preferences_and_multimodules
{
  namespace execution
  {
    ParametersDescription options()
    {
      ParametersDescription driver_opts;
      driver_opts.add (gspc::options::installation());
      driver_opts.add (gspc::options::drts());
      driver_opts.add (gspc::options::logging());
      driver_opts.add (gspc::options::scoped_rifd());

      return driver_opts;
    }
  }

  WorkflowResult execute (Workflow workflow, Parameters parameters)
  {
    auto const preferences (workflow.preferences());
    auto const num_workers_per_target (workflow.num_workers_per_target());

    auto const preferences_and_multimodules_installation_path
      (fhg::util::executable_path().parent_path().parent_path());

    // Create an object containing information about the GPI-Space installation used.
    gspc::installation installation (parameters);

    // Start remote interface daemons on the provided hosts, used for starting
    // the GPI-Space components of the runtime system.
    gspc::scoped_rifds rifds
      ( gspc::rifd::strategy {parameters}
      , gspc::rifd::hostnames {parameters}
      , gspc::rifd::port {parameters}
      , installation
      );

    workflow.num_nodes (rifds.hosts().size());

    // Set the application search path to point to the location where
    // the shared libraries containing task implementation are installed
    gspc::set_application_search_path
      (parameters, preferences_and_multimodules_installation_path / "lib");

    // Create a worker topology to be respected on each node when starting workers.
    // Typically, for each target in the list of preferences, a number of workers
    // (specified by the user) having that capability should be spawned by the
    // bootstrapping mechanism. In the simplest form, this can be specified as
    // an array of concatenated strings of type "<target>:<number_of_workers>",
    // separated by a space.
    std::string const topology
      { fmt::format
        ( "{}:{} {}:{} {}:{}"
        , preferences[0]
        , num_workers_per_target[0]
        , preferences[1]
        , num_workers_per_target[1]
        , preferences[2]
        , num_workers_per_target[2]
        )
      };

    // Start the runtime system's components with the given topology and parameters
    // on the set of specified entry points corresponding to the started rif daemons.
    gspc::scoped_runtime_system drts
      ( parameters
      , installation
      , topology
      , rifds.entry_points()
      );

    // Submit an workflow to the runtime system and wait for the result.
    return gspc::client {drts}.put_and_run
      ( preferences_and_multimodules_installation_path
        / "pnet"
        / "preferences_and_multimodules.pnet"
      , {{"num_tasks", workflow.num_tasks()}}
      );
  }
}
