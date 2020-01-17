#pragma once

#include <gspc/Task.hpp>

namespace gspc
{
  //! between Scheduler and Worker
  struct Job
  {
    //! \todo multiple tasks per job!?
    Task task;

    template<typename Archive>
      void serialize (Archive& ar, unsigned int);
  };
}

#include <gspc/Job.ipp>
