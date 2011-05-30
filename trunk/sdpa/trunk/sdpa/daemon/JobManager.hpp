/*
 * =====================================================================================
 *
 *       Filename:  JobManager.hpp
 *
 *    Description:  Job manager
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
#ifndef SDPA_DAEMON_JOB_MANAGER_HPP
#define SDPA_DAEMON_JOB_MANAGER_HPP 1

#include <sdpa/common.hpp>
#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/exceptions.hpp>
#include <sdpa/serialize/unordered_map.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>

#include <sdpa/engine/IWorkflowEngine.hpp>

namespace sdpa { namespace tests { class DaemonFSMTest_SMC; class DaemonFSMTest_BSC;}}

namespace sdpa { namespace daemon {

    typedef boost::unordered_map<sdpa::job_id_t, preference_t> preference_map_t;

  class JobManager  {
  public:
	  typedef sdpa::shared_ptr<JobManager> ptr_t;
	  typedef boost::recursive_mutex mutex_type;
	  typedef boost::unique_lock<mutex_type> lock_type;
	  typedef boost::unordered_map<sdpa::job_id_t, sdpa::daemon::Job::ptr_t> job_map_t;
	  //typedef std::map<sdpa::job_id_t, sdpa::daemon::Job::ptr_t> job_map_t;
	  typedef job_map_t::iterator iterator;

	  iterator begin() { return job_map_.begin(); }
	  iterator end() { return job_map_.end(); }

	  JobManager();
	  virtual ~JobManager();
	  virtual Job::ptr_t& findJob(const sdpa::job_id_t& ) throw (JobNotFoundException) ;
	  // virtual Job::ptr_t getJob();
	  virtual void addJob(const sdpa::job_id_t&, const Job::ptr_t& ) throw(JobNotAddedException) ;
	  virtual void deleteJob(const sdpa::job_id_t& ) throw(JobNotDeletedException) ;
	  void markJobForDeletion(const sdpa::job_id_t& job_id, const Job::ptr_t& pJob) throw(JobNotMarkedException);
	  std::vector<sdpa::job_id_t> getJobIDList();
	  void updateJobInfo(IComm*);

	  unsigned int numberExtJobs();

	  void addJobPreferences( const sdpa::job_id_t&, const preference_t& ) throw (JobNotFoundException);
	  const preference_t& getJobPreferences(const sdpa::job_id_t& jobId) const throw (NoJobPreferences);

	  std::string print() const;
	  size_t number_of_jobs() const { return job_map_.size(); }

	  void waitForFreeSlot();
          bool slotAvailable() const;

          void resubmitJobsAndResults(IComm* );

	  template <class Archive>
	  void serialize(Archive& ar, const unsigned int)
	  {
		  ar & BOOST_SERIALIZATION_NVP(job_map_);
		  ar & BOOST_SERIALIZATION_NVP(job_map_marked_for_del_);
	  }

	  /*
      template<class Archive>
      void save(Archive & ar, const unsigned int version) const
	  {
		  // note, version is always the latest when saving
		  ar  & job_map_;
		  ar  & job_map_marked_for_del_;
	  }

	  template<class Archive>
	  void load(Archive & ar, const unsigned int version)
	  {
		  ar & job_map_;
		  ar & job_map_marked_for_del_;
	  }

	  BOOST_SERIALIZATION_SPLIT_MEMBER()
	  */


	  friend class boost::serialization::access;
	  // only for testing purposes!
	  friend class sdpa::tests::DaemonFSMTest_SMC;
	  friend class sdpa::tests::DaemonFSMTest_BSC;

	  job_map_t job_map_;
  protected:
	  SDPA_DECLARE_LOGGER();

	  job_map_t job_map_marked_for_del_;
	  mutable mutex_type mtx_;
          boost::condition_variable_any free_slot_;
	  preference_map_t job_preferences_;
  };
}}

#endif
