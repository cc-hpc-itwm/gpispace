// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <preferences_and_multimodules/workflow.hpp>
#include <preferences_and_multimodules/WorkflowResult.hpp>

#include <drts/scoped_rifd.hpp>

#include <we/type/value.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <fmt/core.h>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

namespace preferences_and_multimodules
{
  Workflow::Workflow (Parameters const& args)
    : _num_tasks (args.at ("num-tasks").as<long>())
    , _preferences {"FPGA", "GPU", "CPU"}
    , _num_workers_per_target
      {args.at ("num-workers-per-target").as<std::vector<unsigned long>>()}
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

  long Workflow::num_tasks() const
  {
    return _num_tasks;
  }

  std::size_t Workflow::num_nodes() const
  {
    return _num_nodes;
  }

  void Workflow::num_nodes (std::size_t num_nodes)
  {
    _num_nodes = num_nodes;
  }

  std::array<std::string, NUM_PREFERENCES> Workflow::preferences() const
  {
    return _preferences;
  }
  std::vector<unsigned long> Workflow::num_workers_per_target() const
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
      return fmt::format
        ( "Error: user preferences were not respected! "
          "There were expected:\n"
          "{} implementations of type {}, "
          "{} implementations of type {} and "
          "{} implementations of type {}. \n"
          "Instead of this, got {}, {}and {}, respectively!\n"
        , n_0
        , preferences[0]
        , n_1
        , preferences[1]
        , n_2
        , preferences[2]
        , num_implementations_of_type[0]
        , num_implementations_of_type[1]
        , num_implementations_of_type[2]
        );
    }

    std::string operator()
      ( unsigned long num_workers_per_target
      , std::string const& preference
      , unsigned long num_implementations_of_type
      )
    {
      return fmt::format
        ( "Error: expected to have at least {} implementations "
          "of type {} run, but got only {}!"
        , num_workers_per_target
        , preference
        , num_implementations_of_type
        );
    }

  } Error;

  void Workflow::process (WorkflowResult result)
  {
    std::transform
      ( _num_workers_per_target.begin()
      , _num_workers_per_target.end()
      , _num_workers_per_target.begin()
      , [&] (unsigned long n) -> unsigned long
        {
          return n * _num_nodes;
        }
      );

    // check if the workflow finished correctly
    we::type::literal::control const done;
    if ( ! (result.at<we::type::literal::control> ("done") == done)
       )
    {
      throw std::runtime_error ("The workflow finished abnormally!");
    }

    // check if exactly one task impementation was executed
    auto const implementations
      (result.at_all<std::string> (std::string {"implementation"}, _num_tasks));

    // compute the number of implementations of each type executed.
    std::array<unsigned long, NUM_PREFERENCES> num_implementations_of_type
      {0, 0, 0};
    std::for_each ( implementations.begin()
                  , implementations.end()
                    , [&num_implementations_of_type, this]
                    (auto const& implementation)
                    {
                      for (unsigned int i{0}; i < _preferences.size(); ++i)
                      {
                        if (implementation == _preferences[i])
                        {
                          num_implementations_of_type[i]++;
                        }
                      }
                    }
                  );

    if (_num_tasks <= _num_workers_per_target[0])
    {
      // if the number of tasks doesn't exceed the number of workers dedicated to
      // the most preferred target (i.e. preferences[0]), it is expected that for
      // all these tasks the most preferred target implementation is executed
      // and no otther implementation is executed.
      if ( num_implementations_of_type[0]  != _num_tasks
         || num_implementations_of_type[1] != 0
         || num_implementations_of_type[2] != 0
         )
      {
        throw std::runtime_error
          (Error (_num_tasks, 0, 0, _preferences, num_implementations_of_type));
      }
    }
    else if (_num_tasks <= _num_workers_per_target[0]
                         + _num_workers_per_target[1]
            )
    {
      // else, if the number of tasks doesn't exceed the number of workers
      // dedicated to the first two most preferred targets (i.e. preferences[0]
      // and preferences[1]), it is expected that for all these tasks the most
      // two preferred target implementations are executed and no other
      // implementation of a different type is executed.
      if ( num_implementations_of_type[0] != _num_workers_per_target[0]
         || num_implementations_of_type[1] != (_num_tasks - _num_workers_per_target[0])
         || num_implementations_of_type[2] != 0
         )
      {
        throw std::runtime_error
          ( Error ( _num_workers_per_target[0]
                  , _num_tasks - _num_workers_per_target[0]
                  , 0
                  , _preferences
                  , num_implementations_of_type
                  )
          );
      }
    }
    else if ( _num_tasks <= _num_workers_per_target[0]
                          + _num_workers_per_target[1]
                          + _num_workers_per_target[2]
            )
    {
      // else, if the number of tasks doesn't exceed the number of workers
      // dedicated to the preferred targets (i.e. preferences[0], preferences[1]
      // and preferences[2]), it is expected that for all these tasks all
      // target implementations are executed.
      if ( num_implementations_of_type[0] != _num_workers_per_target[0]
         || num_implementations_of_type[1] != _num_workers_per_target[1]
         || num_implementations_of_type[2]
         != (_num_tasks - _num_workers_per_target[0] - _num_workers_per_target[1])
         )
      {
        throw std::runtime_error
          ( Error ( _num_workers_per_target[0]
                  , _num_workers_per_target[1]
                  , _num_tasks - _num_workers_per_target[0] - _num_workers_per_target[1]
                  , _preferences
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
      for (auto i {0}; i < _num_workers_per_target.size(); ++i)
      {
        if (num_implementations_of_type[i] < _num_workers_per_target[i])
        {
          throw std::runtime_error
            ( Error ( _num_workers_per_target[i]
                    , _preferences[i]
                    , num_implementations_of_type[i]
                    )
            );
        }
      }
    }
  }
}
