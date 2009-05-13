#ifndef SDPA_MESSAGES_QUERYJOBSTATUSEVENT_HPP
#define SDPA_MESSAGES_QUERYJOBSTATUSEVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>

namespace sdpa {
  namespace events {
    class QueryJobStatusEvent : public sdpa::events::JobEvent {
      public:
        typedef std::tr1::shared_ptr<QueryJobStatusEvent> Ptr;
        QueryJobStatusEvent( ) {}
        ~QueryJobStatusEvent( ) {}
    };
  }
}


#endif
