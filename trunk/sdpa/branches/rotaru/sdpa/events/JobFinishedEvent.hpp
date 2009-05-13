#ifndef SDPA_MESSAGES_JOBFINISHEDEVENT_HPP
#define SDPA_MESSAGES_JOBFINISHEDEVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>

namespace sdpa {
  namespace events {
    class JobFinishedEvent : public sdpa::events::JobEvent {
      public:
        typedef std::tr1::shared_ptr<JobFinishedEvent> Ptr;
        JobFinishedEvent(){}
        ~JobFinishedEvent(){}
    };
  }
}


#endif
