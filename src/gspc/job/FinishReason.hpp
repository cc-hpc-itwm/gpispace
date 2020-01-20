#pragma once

#include <gspc/job/finish_reason/Cancelled.hpp>
#include <gspc/job/finish_reason/Finished.hpp>
#include <gspc/job/finish_reason/WorkerFailure.hpp>

#include <boost/variant.hpp>

namespace gspc
{
  namespace job
  {
    //! \todo Is worker failure a finish though?
    //! \todo monitor workers to not wait forever on task result
    using FinishReason = boost::variant < finish_reason::Finished
                                        , finish_reason::WorkerFailure
                                        , finish_reason::Cancelled
                                        >;
  }
}
