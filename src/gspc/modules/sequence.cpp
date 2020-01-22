#include <gspc/module_api.hpp>

#include <gspc/SequenceWorkflowEngine.hpp>
#include <gspc/serialization.hpp>

#include <stdexcept>

namespace sequence_so
{
  gspc::SequenceOutput identity (gspc::SequenceInput input)
  {
    return input;
  }
}

//! \todo automatically generate wrapper
namespace autogen
{
  gspc::task::result::Success::Output identity (gspc::Task::Input input)
  {
    return gspc::bytes_save
      (sequence_so::identity (gspc::bytes_load<gspc::SequenceInput> (input)));
  }
}

extern "C" FHG_UTIL_DLLEXPORT gspc::ModuleFunctions const gspc_module_functions
  = {{"identity", &autogen::identity}};
