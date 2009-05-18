#ifndef SDPA_MESSAGES_JOBEVENT_HPP
#define SDPA_MESSAGES_JOBEVENT_HPP 1

#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/Job.hpp>
#include <sstream>

namespace sdpa {
  namespace events {
    class JobEvent : public sdpa::events::SDPAEvent {
      public:
        typedef std::tr1::shared_ptr<JobEvent> Ptr;
        JobEvent(): sdpa::events::SDPAEvent("",""){}
        ~JobEvent(){}
        Job::job_id_t GetJobID() {return m_nJobID;}
        void SetJobID( Job::job_id_t JobID ) { m_nJobID = JobID;}

        std::string str() const {
            std::ostringstream oss(std::ostringstream::out);
            oss << "empty string";
            return oss.str();
        }

      private:
    	  Job::job_id_t m_nJobID;
    };
  }
}

#endif
