#ifndef SDPA_MESSAGES_JOBSTATUSANSWEREVENT_HPP
#define SDPA_MESSAGES_JOBSTATUSANSWEREVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>

namespace sdpa {
  namespace events {
    class JobStatusAnswerEvent : public sdpa::events::JobEvent {
      public:
        typedef std::tr1::shared_ptr<JobStatusAnswerEvent> Ptr;
        JobStatusAnswerEvent(){}
        ~JobStatusAnswerEvent(){}
    };
  }
}


#endif
