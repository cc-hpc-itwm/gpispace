#include <gspc/workflow_engine/State.hpp>

#include <ios>

namespace gspc
{
  namespace workflow_engine
  {
    std::ostream& operator<< (std::ostream& os, State const& s)
    {
      return os
        << "workflow_finished: "
        << std::boolalpha << s.workflow_finished << "\n"
        << "processing_state:\n{\n" << s.processing_state << "\n}"
        ;
    }
  }
}