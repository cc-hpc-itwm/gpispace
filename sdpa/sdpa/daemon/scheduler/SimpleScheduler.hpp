// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef SDPA_SimpleScheduler_HPP
#define SDPA_SimpleScheduler_HPP 1

#include <sdpa/daemon/scheduler/SchedulerBase.hpp>

namespace sdpa {
  namespace daemon {
    class SimpleScheduler : public SchedulerBase {
    public:
      typedef boost::shared_ptr<SimpleScheduler> ptr_t;
      SimpleScheduler(GenericDaemon*);

      virtual void assignJobsToWorkers();
      virtual void rescheduleJob(const sdpa::job_id_t& job_id );
    };
  }
}

#endif
