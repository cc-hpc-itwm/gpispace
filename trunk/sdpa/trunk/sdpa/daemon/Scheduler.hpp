/*
 * =====================================================================================
 *
 *       Filename:  Scheduler.hpp
 *
 *    Description:  Defines a basic scheduler interface
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#ifndef SDPA_SCHEDULER_HPP
#define SDPA_SCHEDULER_HPP 1

#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/Worker.hpp>
#include <sdpa/daemon/exceptions.hpp>
#include <boost/serialization/access.hpp>
#include <we/we.hpp>

namespace sdpa {
namespace daemon {
  class Scheduler {
  public:

	 typedef sdpa::shared_ptr<Scheduler> ptr_t;

	 virtual const Worker::worker_id_t& findWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException) = 0;

	 virtual const Worker::ptr_t& findWorker(const Worker::worker_id_t&  ) throw(WorkerNotFoundException) = 0;

         virtual const sdpa::job_id_t getNextJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &last_job_id) throw (NoJobScheduledException, WorkerNotFoundException) =0;
	 virtual void deleteWorkerJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &job_id ) throw (JobNotDeletedException, WorkerNotFoundException) = 0;

	 virtual void acknowledgeJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id) throw(WorkerNotFoundException, JobNotFoundException) = 0;

	 virtual void addWorker( const Worker::worker_id_t& workerId, unsigned int rank ) throw (WorkerAlreadyExistException) = 0;
	 virtual void delWorker( const Worker::worker_id_t& workerId) throw (WorkerNotFoundException) = 0;

	 virtual size_t numberOfWorkers() = 0;

	 virtual void schedule(sdpa::job_id_t& jobId) = 0;
	 virtual bool schedule_to(const sdpa::job_id_t& jobId, unsigned int rank, const we::preference_t& job_pref ) = 0;

	 virtual void start()=0;
	 virtual void stop()=0;
	 virtual void run()=0;
	 virtual void print()=0;

	 friend class boost::serialization::access;
	 template<class Archive>
		void serialize(Archive&, const unsigned int /* file version */){}
  };
}}


BOOST_SERIALIZATION_ASSUME_ABSTRACT( sdpa::daemon::Scheduler )

#endif
