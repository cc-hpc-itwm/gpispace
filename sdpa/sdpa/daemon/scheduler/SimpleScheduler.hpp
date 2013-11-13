// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef SDPA_SimpleScheduler_HPP
#define SDPA_SimpleScheduler_HPP 1

#include <sdpa/daemon/scheduler/SchedulerBase.hpp>

using namespace sdpa::events;
using namespace std;

namespace sdpa {
  namespace daemon {
    class SimpleScheduler : public SchedulerBase {

    public:
    SimpleScheduler(sdpa::daemon::IAgent* pCommHandler = NULL,  bool bUseReqModel = true);

    virtual ~SimpleScheduler();

    void assignJobsToWorkers();
    void rescheduleJob(const sdpa::job_id_t& job_id );
    void releaseReservation(const sdpa::job_id_t& jobId);
    void workerFinished(const worker_id_t& wid, const job_id_t& jid);
    void workerFailed(const worker_id_t& wid, const job_id_t& jid);
    void workerCanceled(const worker_id_t& wid, const job_id_t& jid);
    bool allPartialResultsCollected(const job_id_t& jid);
    bool groupFinished(const sdpa::job_id_t& jid);

    bool postRequest( bool );
    void checkRequestPosted();

    template <class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
      ar & boost::serialization::base_object<SchedulerBase>(*this);
    }

    private:
      SDPA_DECLARE_LOGGER();
    };
  }
}

#endif
