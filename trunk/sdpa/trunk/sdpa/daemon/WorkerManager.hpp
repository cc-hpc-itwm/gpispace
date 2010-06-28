/*
 * =====================================================================================
 *
 *       Filename:  WorkerManager.hpp
 *
 *    Description:  Worker manager
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
#ifndef SDPA_DAEMON_WORKER_MANAGER_HPP
#define SDPA_DAEMON_WORKER_MANAGER_HPP 1

#include <sdpa/daemon/Worker.hpp>
#include <sdpa/daemon/exceptions.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>

namespace sdpa { namespace tests { class DaemonFSMTest_SMC; class DaemonFSMTest_BSC;}}

namespace sdpa { namespace daemon {
  class WorkerManager  {
  public:
	  typedef sdpa::shared_ptr<WorkerManager> ptr_t;
	  typedef boost::recursive_mutex mutex_type;
	  typedef boost::unique_lock<mutex_type> lock_type;
	  typedef std::map<Worker::worker_id_t, Worker::ptr_t> worker_map_t;
	  typedef std::map<unsigned int, Worker::worker_id_t> rank_map_t;

	  WorkerManager();
	  virtual ~WorkerManager();

	  Worker::ptr_t &findWorker(const Worker::worker_id_t& worker_id) throw (WorkerNotFoundException);
	  Worker::ptr_t &findWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException);

	  void addWorker( const Worker::worker_id_t& workerId, unsigned int rank ) throw (WorkerAlreadyExistException);
	  Worker::ptr_t& getNextWorker() throw (NoWorkerFoundException);
	  unsigned int getLeastLoadedWorker() throw (NoWorkerFoundException);

	  sdpa::job_id_t getNextJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &last_job_id) throw (NoJobScheduledException);
	  void dispatchJob(const sdpa::job_id_t& jobId);
	  size_t numberOfWorkers() { return worker_map_.size(); }

	  void balanceWorkers();

	  //only for testing purposes!
	  friend class sdpa::tests::DaemonFSMTest_SMC;
	  friend class sdpa::tests::DaemonFSMTest_BSC;

	  template <class Archive>
	  void serialize(Archive& ar, const unsigned int file_version )
	  {
		  ar & worker_map_;
	  }

	  friend class boost::serialization::access;

	  void print() {
		  for( worker_map_t::iterator it = worker_map_.begin(); it!=worker_map_.end(); it++)
			 (*it).second->print();
	  }

	  worker_map_t worker_map_;
	  rank_map_t rank_map_;
  protected:
	  SDPA_DECLARE_LOGGER();
	  worker_map_t::iterator iter_last_worker_;

	  Worker::JobQueue common_queue_;

	  mutable mutex_type mtx_;
  };
}}

#endif
