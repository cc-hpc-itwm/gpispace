#pragma once

#include <gspc/workflow_engine/ProcessingState.hpp>

#include <vector>

namespace gspc
{
  namespace workflow_engine
  {
    struct State
    {
      //! \todo ctor
      // private:
      std::vector<char> engine_specific;

      ProcessingState processing_state;

      template<typename Archive>
        void serialize (Archive&, unsigned int);
    };
  }
}

#include <gspc/workflow_engine/State.ipp>
