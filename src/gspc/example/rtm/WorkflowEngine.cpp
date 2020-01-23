#include <gspc/example/rtm/WorkflowEngine.hpp>

#include <gspc/serialization.hpp>

#include <util-generic/serialization/boost/filesystem/path.hpp>

#include <boost/serialization/optional.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/vector.hpp>

namespace gspc
{
  namespace rtm
  {
    print_task::print_task (Task const& task) : _task (task) {}

    std::ostream& print_task::operator() (std::ostream& os) const
    {
      if (_task.symbol == "load")
      {
        auto input (bytes_load<LoadInput> (_task.input));

        return os << "Load (" << input.shot << ")";
      }
      else if (_task.symbol == "process")
      {
        auto input (bytes_load<ProcessInput> (_task.input));

        return os << "Process (" << input.shot << ")";
      }
      else if (_task.symbol == "reduce")
      {
        auto input (bytes_load<ReduceInput> (_task.input));

        return os << "Reduce ("
                  << fhg::util::print_container ("{", ", ", "}", input.lhs)
                  << " + "
                  << fhg::util::print_container ("{", ", ", "}", input.rhs)
                  << ")";
      }
      else if (_task.symbol == "store")
      {
        auto input (bytes_load<StoreInput> (_task.input));

        return os << "Store ("
                  << fhg::util::print_container ("{", ", ", "}", input.result)
                  << ")";
      }
      else
      {
        throw std::logic_error ("task with unknown symbol " + _task.symbol);
      }
    }

    WorkflowEngine::WorkflowEngine
      ( boost::filesystem::path module
      , Parameter parameter
      )
    {
      _workflow_state.module = module;
      _workflow_state.parameter = parameter;

      for (Shot shot {0}; shot < parameter.number_of_shots; ++shot)
      {
        _workflow_state.front.emplace_back
          (LoadInput {_workflow_state.parameter, shot});
      }
    }

    bool WorkflowEngine::workflow_finished() const
    {
      return _workflow_state.front.empty();
    }

    Task WorkflowEngine::at (task::ID task_id) const
    {
      return _processing_state.at (task_id);
    }

    boost::optional<PartialResult> WorkflowEngine::final_result() const
    {
      return _workflow_state.final_result;
    }

    template<typename Archive>
      void WorkflowEngine::WorkflowState::serialize
        (Archive& ar, unsigned int)
    {
      ar & module;
      ar & parameter;
      ar & front;
      ar & partial_results;
      ar & final_result;
    }

    workflow_engine::State WorkflowEngine::state() const
    {
      return { bytes_save (_workflow_state)
             , workflow_finished()
             , _processing_state
             };
    }
    WorkflowEngine::WorkflowEngine (workflow_engine::State state)
      : _workflow_state (bytes_load<WorkflowState> (state.engine_specific))
      , _processing_state (state.processing_state)
    {
      if (state.workflow_finished != workflow_finished())
      {
        throw std::logic_error ("INCONSISTENCY: finished or not!?");
      }
    }

    namespace
    {
      template<typename T>
        T pop_any (std::vector<T>& xs)
      {
        auto x (std::move (xs.back()));
        xs.pop_back();
        return x;
      }
    }

    boost::variant<Task, bool> WorkflowEngine::extract()
    {
      if (_processing_state.has_retry_task())
      {
        return _processing_state.retry_task();
      }

      if (workflow_finished())
      {
        return !_processing_state.has_extracted_tasks();
      }

      return fhg::util::visit<Task>
        ( pop_any (_workflow_state.front)
        , [&] (LoadInput input)
          {
            return _processing_state.extract
              ("load", bytes_save (input), _workflow_state.module, "load");
          }
        , [&] (ProcessInput input)
          {
            //! \todo coallocation
            return _processing_state.extract
              ("node", bytes_save (input), _workflow_state.module, "process");
          }
        , [&] (ReduceInput input)
          {
            return _processing_state.extract
              ("socket", bytes_save (input), _workflow_state.module, "reduce");
          }
        , [&] (StoreInput input)
          {
            return _processing_state.extract
              ("store", bytes_save (input), _workflow_state.module, "store");
          }
        );
    }

    interface::WorkflowEngine::InjectResult
      WorkflowEngine::inject (task::ID id, task::Result result)
    {
      _processing_state.inject
        ( std::move (id)
        , std::move (result)
        , [&] (Task const& input_task, task::result::Success const& success)
          {
            auto store
              ( [&] (auto result)
                {
                  _workflow_state.front.emplace_back
                    (StoreInput {_workflow_state.parameter, result});
                }
              );

            if (input_task.symbol == "load")
            {
              auto output (bytes_load<LoadOutput> (success.output));
              // auto input (bytes_load<LoadInput> (input_task.input));

              _workflow_state.front.emplace_back
                (ProcessInput {_workflow_state.parameter, output.shot});
            }
            else if (input_task.symbol == "process")
            {
              auto output (bytes_load<ProcessOutput> (success.output));
              // auto input (bytes_load<ProcessInput> (input_task.input));

              _workflow_state.partial_results.emplace_back (output.result);

              store (output.result);
            }
            else if (input_task.symbol == "reduce")
            {
              auto output (bytes_load<ReduceOutput> (success.output));
              // auto input (bytes_load<ReduceInput> (input_task.input));

              if ( output.result.size()
                 < _workflow_state.parameter.number_of_shots
                 )
              {
                _workflow_state.partial_results.emplace_back (output.result);
              }

              store (output.result);
            }
            else if (input_task.symbol == "store")
            {
              auto output (bytes_load<StoreOutput> (success.output));
              // auto input (bytes_load<StoreInput> (input_task.input));

              if (  output.result.size()
                 == _workflow_state.parameter.number_of_shots
                 )
              {
                _workflow_state.final_result = output.result;
              }
            }
            else
            {
              throw std::logic_error
                ("task with unknown symbol " + input_task.symbol);
            }

            while (_workflow_state.partial_results.size() >= 2)
            {
              auto const lhs (pop_any (_workflow_state.partial_results));
              auto const rhs (pop_any (_workflow_state.partial_results));

              _workflow_state.front.emplace_back
                (ReduceInput {_workflow_state.parameter, lhs, rhs});
            }
          }
        );

      return {};
    }
  }
}
