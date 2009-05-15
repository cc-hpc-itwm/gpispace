#ifndef SDPA_MESSAGES_JOBEVENT_HPP
#define SDPA_MESSAGES_JOBEVENT_HPP 1

#include <sdpa/messages/SDPAEvent.hpp>
#include <sdpa/Job.hpp>

namespace sdpa {
  namespace events {
    class JobEvent : public sdpa::SDPAEvent {
      public:
        typedef std::tr1::shared_ptr<JobEvent> Ptr;
        JobEvent():SDPAEvent("",""){}
        ~JobEvent(){}
        Job::job_id_t GetJobID() {return m_nJobID;}
        void SetJobID( Job::job_id_t JobID ) { m_nJobID = JobID;}
      private:
    	  Job::job_id_t m_nJobID;
    };
  }
}

#endif
