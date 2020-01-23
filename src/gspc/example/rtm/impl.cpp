#include <gspc/interface/module_api.hpp>

#include <gspc/example/rtm/WorkflowEngine.hpp>
#include <gspc/serialization.hpp>

#include <iostream>
#include <random>
#include <stdexcept>

namespace gspc
{
  namespace rtm
  {
    namespace impl
    {
      namespace
      {
        std::random_device random_device;
        std::uniform_real_distribution<double> distribution;

        void maybe_failure (double probability)
        {
          if (distribution (random_device) < probability)
          {
            throw std::runtime_error ("failed");
          }
        }
      }

      LoadOutput load (LoadInput input)
      {
        maybe_failure (input.parameter.probability_of_failure.load);

        return {input.shot};
      }

      ProcessOutput process (ProcessInput input)
      {
        maybe_failure (input.parameter.probability_of_failure.process);

        return {{input.shot}};
      }

      ReduceOutput reduce (ReduceInput input)
      {
        maybe_failure (input.parameter.probability_of_failure.reduce);

        PartialResult reduced (input.lhs);
        reduced.insert (input.rhs.begin(), input.rhs.end());

        return {reduced};
      }

      StoreOutput store (StoreInput input)
      {
        maybe_failure (input.parameter.probability_of_failure.store);

        return {input.result};
      }
    }

    namespace autogen
    {
      std::vector<char> load (std::vector<char> input)
      {
        return bytes_save (impl::load (bytes_load<LoadInput> (input)));
      }
      std::vector<char> process (std::vector<char> input)
      {
        return bytes_save (impl::process (bytes_load<ProcessInput> (input)));
      }
      std::vector<char> reduce (std::vector<char> input)
      {
        return bytes_save (impl::reduce (bytes_load<ReduceInput> (input)));
      }
      std::vector<char> store (std::vector<char> input)
      {
        return bytes_save (impl::store (bytes_load<StoreInput> (input)));
      }
    }
  }
}

extern "C" FHG_UTIL_DLLEXPORT gspc::ModuleFunctions const gspc_module_functions
  = { {"load", &gspc::rtm::autogen::load}
    , {"process", &gspc::rtm::autogen::process}
    , {"reduce", &gspc::rtm::autogen::reduce}
    , {"store", &gspc::rtm::autogen::store}
    };
