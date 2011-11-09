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
#include <boost/unordered_map.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <sdpa/engine/IWorkflowEngine.hpp>

namespace sdpa { namespace tests { class DaemonFSMTest_SMC; class DaemonFSMTest_BSC;}}

namespace sdpa { namespace daemon {
  class WorkerManager  {
  public:
      typedef sdpa::shared_ptr<WorkerManager> ptr_t;
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;
      typedef boost::unordered_map<Worker::worker_id_t, Worker::ptr_t> worker_map_t;
      typedef boost::unordered_map<unsigned int, Worker::worker_id_t> rank_map_t;
      typedef boost::unordered_map<sdpa::job_id_t, Worker::worker_id_t > owner_map_t;

      WorkerManager();
      virtual ~WorkerManager();

      Worker::ptr_t& findWorker(const Worker::worker_id_t& worker_id) throw (WorkerNotFoundException);
      const Worker::worker_id_t& findWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException);
      const Worker::worker_id_t& findAcknowlegedWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException);

      void addWorker( const Worker::worker_id_t& workerId, unsigned int capacity,
    		          const capabilities_set_t& cpbset = capabilities_set_t(),
    		          const unsigned int& agent_rank = 0,
    		          const sdpa::worker_id_t& agent_uuid = "" ) throw (WorkerAlreadyExistException);

      void delWorker( const Worker::worker_id_t& workerId) throw (WorkerNotFoundException);

      bool addCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset);
      virtual void removeCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset)  throw (WorkerNotFoundException);
      virtual void getCapabilities(const std::string& agentName, sdpa::capabilities_set_t& cpbset);

     // void getListOfRegisteredRanks( std::vector<unsigned int>& );

      void removeWorkers() { worker_map_.clear(); }
      const Worker::ptr_t& getNextWorker() throw (NoWorkerFoundException);
      worker_id_t getLeastLoadedWorker() throw (NoWorkerFoundException, AllWorkersFullException);

      const sdpa::job_id_t stealWork(const Worker::worker_id_t& worker_id) throw (NoJobScheduledException);

      Worker::ptr_t getBestMatchingWorker( const requirement_list_t& listJobReq ) throw (NoWorkerFoundException);

      const sdpa::job_id_t getNextJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &last_job_id) throw (NoJobScheduledException, WorkerNotFoundException);
      void dispatchJob(const sdpa::job_id_t& jobId);
      void delete_job(const sdpa::job_id_t& jobId);
      void deleteWorkerJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &job_id ) throw (JobNotDeletedException, WorkerNotFoundException);

      Worker::worker_id_t getOwnerId(const sdpa::job_id_t& job_id) throw (JobNotAssignedException);
      void make_owner(const sdpa::job_id_t& job_id, const worker_id_t& worker_id );

      size_t numberOfWorkers() { return worker_map_.size(); }
      void getWorkerList(std::list<std::string>& workerList);
      void balanceWorkers();
      const Worker::worker_id_t& worker(unsigned int rank) throw (NoWorkerFoundException);

      bool has_job(const sdpa::job_id_t& job_id);

      //only for testing purposes!
      friend class sdpa::tests::DaemonFSMTest_SMC;
      friend class sdpa::tests::DaemonFSMTest_BSC;

      template <class Archive>
      void serialize(Archive& ar, const unsigned int)
      {
          ar & worker_map_;
          ar & rank_map_;
          ar & owner_map_;
          //ar & common_queue_;
      }

      friend class boost::serialization::access;

      void print()
      {
          if(!common_queue_.empty())
          {
        	  SDPA_LOG_DEBUG("The content of the common queue is: ");
        	  common_queue_.print();
          }
          else
        	  SDPA_LOG_DEBUG("No job without preferences available!");

          if( worker_map_.begin() == worker_map_.end() )
        	  SDPA_LOG_DEBUG("The worker manager has NO worker! ");
          else
          {
              SDPA_LOG_DEBUG("The worker manager has workers! ");
              for( worker_map_t::iterator it = worker_map_.begin(); it!=worker_map_.end(); it++)
                (*it).second->print();
          }
      }


      const worker_map_t&  worker_map() const { return worker_map_; }
      const rank_map_t& rank_map() const { return rank_map_; }
      const owner_map_t& owner_map() const { return owner_map_; }
      owner_map_t& owner_map() { return owner_map_; }

protected:
      worker_map_t  worker_map_;
      rank_map_t    rank_map_;
      owner_map_t   owner_map_;

      SDPA_DECLARE_LOGGER();
      worker_map_t::iterator iter_last_worker_;

      Worker::JobQueue common_queue_;

      mutable mutex_type mtx_;
  };
}}

#endif
