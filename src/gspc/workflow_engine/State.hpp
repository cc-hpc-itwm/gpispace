#pragma once

#include <gspc/workflow_engine/ProcessingState.hpp>

#include <ostream>
#include <vector>

namespace gspc
{
  namespace workflow_engine
  {
    struct State
    {
      friend std::ostream& operator<< (std::ostream&, State const&);

      //! \todo ctor
      // private:
      std::vector<char> engine_specific;
      bool workflow_finished;

      ProcessingState processing_state;
    };
  }
}
