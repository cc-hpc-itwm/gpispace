// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef SDPA_SimpleScheduler_HPP
#define SDPA_SimpleScheduler_HPP 1

#include <sdpa/daemon/scheduler/SchedulerBase.hpp>

namespace sdpa {
  namespace daemon {
    class SimpleScheduler : public SchedulerBase {

    public:
    typedef sdpa::shared_ptr<SimpleScheduler> ptr_t;
    SimpleScheduler(sdpa::daemon::IAgent*);

    void assignJobsToWorkers();
    boost::optional<sdpa::worker_id_t> getAssignedWorker(const sdpa::job_id_t& jid);
    void rescheduleJob(const sdpa::job_id_t& job_id );
    };
  }
}

#endif
