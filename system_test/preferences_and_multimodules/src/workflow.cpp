// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <preferences_and_multimodules/workflow.hpp>
#include <preferences_and_multimodules/WorkflowResult.hpp>

#include <drts/scoped_rifd.hpp>

#include <we/type/value.hpp>

#include <boost/format.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <numeric>
#include <string>
#include <vector>

namespace preferences_and_multimodules
{
  Workflow::Workflow (Parameters const& args)
    : _num_tasks (args.at ("num-tasks").as<long>())
    , _preferences {"FPGA", "GPU", "CPU"}
    , _num_workers_per_target (args.at ("num-workers-per-target").as<std::vector<unsigned long>>())
    , _num_nodes {1}
  {
    if (_preferences.size() != _num_workers_per_target.size())
    {
      throw std::runtime_error
        ("For each preference a number of workers corresponding to that target should be specified!");
    }
  }

  ParametersDescription Workflow::options()
  {
    namespace po = boost::program_options;

    ParametersDescription workflow_opts ("Workflow");
    workflow_opts.add_options()
      ( "num-workers-per-target"
      , ::boost::program_options::value<std::vector<unsigned long>>()->multitoken()->required()
      , "number of workers per target (per host)"
      );

    workflow_opts.add_options()
      ( "num-tasks"
      , ::boost::program_options::value<long>()->required()
      , "number of tasks"
    );

    return workflow_opts;
  }

  long Workflow::get_num_tasks() const
  {
    return _num_tasks;
  }

  std::size_t Workflow::get_num_nodes() const
  {
    return _num_nodes;
  }

  void Workflow::set_num_nodes (std::size_t num_nodes)
  {
    _num_nodes = num_nodes;
  }

  std::array<std::string, NUM_PREFERENCES> Workflow::get_preferences() const
  {
    return _preferences;
  }

  std::vector<unsigned long> Workflow::get_num_workers_per_target() const
  {
    return _num_workers_per_target;
  }

  struct ErrorType
  {
    std::string operator()
      ( unsigned long n_0
      , unsigned long n_1
      , unsigned long n_2
      , std::array<std::string, NUM_PREFERENCES> const& preferences
      , std::array<unsigned long, NUM_PREFERENCES> const& num_implementations_of_type
      )
    {
      return
        ( boost::format
            ("Error: user preferences were not respected! "
             "There were expected:\n"
             "%1% implementations of type %2%, "
             "%3% implementations of type %4% and "
             "%5% implementations of type %6%. \n"
             "Instead of this, got %7%, %8% and %9%, respectively!\n"
            )
        % n_0
        % preferences[0]
        % n_1
        % preferences[1]
        % n_2
        % preferences[2]
        % num_implementations_of_type[0]
        % num_implementations_of_type[1]
        % num_implementations_of_type[2]
        ).str();
    }

    std::string operator()
      ( unsigned long num_workers_per_target
      , std::string const& preference
      , unsigned long num_implementations_of_type
      )
    {
      return
        ( boost::format
            ("Error: expected to have at least %1% implementations "
             "of type %2% run, but got only %3%!"
            )
        % num_workers_per_target
        % preference
        % num_implementations_of_type
        ).str();
    }

  } Error;

  void Workflow::process (WorkflowResult result)
  {
    auto const preferences {get_preferences()};
    auto const num_tasks {get_num_tasks()};
    auto const num_nodes {get_num_nodes()};
    auto num_workers_per_target {get_num_workers_per_target()};

    std::transform
      ( num_workers_per_target.begin()
      , num_workers_per_target.end()
      , num_workers_per_target.begin()
      , [&] (unsigned long n) -> unsigned long
        {
          return n * num_nodes;
        }
      );

    // check if the workflow finished correctly
    we::type::literal::control const done;
    if (!(result.get<we::type::literal::control> ("done") == done))
    {
      throw std::runtime_error ("The workflow finished abnormally!");
    }

    // check if exactly one task impementation was executed
    auto const implementations
      (result.get_all<std::string> (std::string {"implementation"}, num_tasks));

    // compute the number of implementations of each type executed.
    std::array<unsigned long, NUM_PREFERENCES> num_implementations_of_type {0, 0, 0};
    std::for_each ( implementations.begin()
                  , implementations.end()
                  , [&num_implementations_of_type, &preferences]
                    (auto const& implementation)
                    {
                      for (unsigned int i{0}; i < preferences.size(); ++i)
                      {
                        if (implementation == preferences[i])
                        {
                          num_implementations_of_type[i]++;
                        }
                      }
                    }
                  );

    if (num_tasks <= num_workers_per_target[0])
    {
      // if the number of tasks doesn't exceed the number of workers dedicated to
      // the most preferred target (i.e. preferences[0]), it is expected that for
      // all these tasks the most preferred target implementation is executed
      // and no otther implementation is executed.
      if ( num_implementations_of_type[0]  != num_tasks
         || num_implementations_of_type[1] != 0
         || num_implementations_of_type[2] != 0
         )
      {
        throw std::runtime_error
          (Error (num_tasks, 0, 0, preferences, num_implementations_of_type));
      }
    }
    else if (num_tasks <= num_workers_per_target[0]
                        + num_workers_per_target[1])
    {
      // else, if the number of tasks doesn't exceed the number of workers
      // dedicated to the first two most preferred targets (i.e. preferences[0]
      // and preferences[1]), it is expected that for all these tasks the most
      // two preferred target implementations are executed and no other
      // implementation of a different type is executed.
      if ( num_implementations_of_type[0] != num_workers_per_target[0]
         || num_implementations_of_type[1] != (num_tasks - num_workers_per_target[0])
         || num_implementations_of_type[2] != 0
         )
      {
        throw std::runtime_error
          ( Error ( num_workers_per_target[0]
                  , num_tasks - num_workers_per_target[0]
                  , 0
                  , preferences
                  , num_implementations_of_type
                  )
          );
      }
    }
    else if ( num_tasks <= num_workers_per_target[0]
                         + num_workers_per_target[1]
                         + num_workers_per_target[2]
            )
    {
      // else, if the number of tasks doesn't exceed the number of workers
      // dedicated to the preferred targets (i.e. preferences[0], preferences[1]
      // and preferences[2]), it is expected that for all these tasks all
      // target implementations are executed.
      if ( num_implementations_of_type[0] != num_workers_per_target[0]
         || num_implementations_of_type[1] != num_workers_per_target[1]
         || num_implementations_of_type[2]
         != (num_tasks - num_workers_per_target[0] - num_workers_per_target[1])
         )
      {
        throw std::runtime_error
          ( Error ( num_workers_per_target[0]
                  , num_workers_per_target[1]
                  , num_tasks - num_workers_per_target[0] - num_workers_per_target[1]
                  , preferences
                  , num_implementations_of_type
                  )
          );
      }
    }
    else
    {
      // else, if the number of tasks exceed the number of workers, it is
      // expected that all target implementations are executed on all workers
      // ensuring thus that all resources are used in a heterogeneous execution
      // environment.
      for (auto i{0}; i < num_workers_per_target.size(); ++i)
      {
        if (num_implementations_of_type[i] < num_workers_per_target[i])
        {
          throw std::runtime_error
            ( Error ( num_workers_per_target[i]
                    , preferences[i]
                    , num_implementations_of_type[i]
                    )
            );
        }
      }
    }
  }
}
