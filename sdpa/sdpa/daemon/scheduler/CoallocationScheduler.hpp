// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef SDPA_COALLOCSCHED_HPP
#define SDPA_COALLOCSCHED_HPP 1

#include <sdpa/daemon/scheduler/SchedulerBase.hpp>

using namespace sdpa::events;
using namespace std;

namespace sdpa {
  namespace daemon {
    class CoallocationScheduler : public SchedulerBase
    {
    public:
      typedef boost::unordered_map<sdpa::job_id_t, Reservation*> allocation_table_t;

      CoallocationScheduler(sdpa::daemon::IAgent* pCommHandler = NULL,  bool use_request_model=true);
      virtual ~CoallocationScheduler();

      void assignJobsToWorkers();
      void rescheduleJob(const sdpa::job_id_t& job_id );
      void reserveWorker(const sdpa::job_id_t& jobId, const sdpa::worker_id_t& matchingWorkerId, const size_t& cap) throw( WorkerReservationFailed);
      void releaseReservation(const sdpa::job_id_t& jobId);
      void getListNotAllocatedWorkers(sdpa::worker_id_list_t& workerList);
      void printAllocationTable();
      sdpa::job_id_t getAssignedJob(const sdpa::worker_id_t& wid);
      void checkAllocations();

      sdpa::worker_id_list_t getListAllocatedWorkers(const sdpa::job_id_t& jobId);

      void workerFinished(const worker_id_t& wid, const job_id_t& jid);
      void workerFailed(const worker_id_t& wid, const job_id_t& jid);
      void workerCanceled(const worker_id_t& wid, const job_id_t& jid);
      bool allPartialResultsCollected(const job_id_t& jid);
      bool groupFinished(const sdpa::job_id_t& jid);

      friend class boost::serialization::access;

      template <class Archive>
      void serialize(Archive& ar, const unsigned int /* file_version */)
      {
        ar & boost::serialization::base_object<SchedulerBase>(*this);
      }

    private:
      SDPA_DECLARE_LOGGER();
      mutable mutex_type mtx_alloc_table_;
      allocation_table_t allocation_table_;
  };
}}

#endif
