#ifndef SDPA_SCHEDULERAGG_HPP
#define SDPA_SCHEDULERAGG_HPP 1

#include <sdpa/daemon/SchedulerImpl.hpp>

namespace sdpa
{
	namespace daemon
  {
    class AgentScheduler : public SchedulerImpl
    {
    public:
      AgentScheduler(sdpa::daemon::IAgent* pCommHandler = NULL,  bool use_request_model=true)
        : SchedulerImpl(pCommHandler, use_request_model)
      {}

      friend class boost::serialization::access;

      template <class Archive>
        void serialize(Archive& ar, const unsigned int /* file_version */)
      {
        ar & boost::serialization::base_object<SchedulerImpl>(*this);
      }
    };
  }
}

#endif
