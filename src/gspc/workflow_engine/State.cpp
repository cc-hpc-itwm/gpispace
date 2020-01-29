#include <gspc/workflow_engine/State.hpp>

namespace gspc
{
  namespace workflow_engine
  {
    std::ostream& operator<< (std::ostream& os, State const& s)
    {
      return os
        << "processing_state:\n{\n" << s.processing_state << "\n}"
        ;
    }
  }
}
