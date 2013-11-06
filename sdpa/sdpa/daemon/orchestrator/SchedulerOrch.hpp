#ifndef SDPA_SCHEDULERORCH_HPP
#define SDPA_SCHEDULERORCH_HPP 1

#include <sdpa/daemon/SchedulerImpl.hpp>

using namespace sdpa::events;
using namespace std;

namespace sdpa
{
  namespace daemon
  {
    class SchedulerOrch : public SchedulerImpl
    {
    public:
      SchedulerOrch(sdpa::daemon::IAgent* pCommHandler = NULL,  bool bUseReqModel = true)
        : SchedulerImpl(pCommHandler, bUseReqModel)
      {}

      bool postRequest( bool ) { return false; }
      void checkRequestPosted() { /*do nothing*/ }

      friend class boost::serialization::access;

      template <class Archive>
        void serialize(Archive& ar, const unsigned int)
      {
        ar & boost::serialization::base_object<SchedulerImpl>(*this);
      }
    };
  }
}

#endif
