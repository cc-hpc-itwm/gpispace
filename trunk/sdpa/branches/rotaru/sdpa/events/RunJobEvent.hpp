#ifndef SDPA_MESSAGES_RUNJOBEVENT
#define SDPA_MESSAGES_RUNJOBEVENT 1

#include <sdpa/events/JobEvent.hpp>

namespace sdpa {
  namespace events {
    class RunJobEvent : public sdpa::events::JobEvent {
      public:
        typedef std::tr1::shared_ptr<RunJobEvent> Ptr;
        RunJobEvent(){}
        ~RunJobEvent(){}
    };
  }
}


#endif
