#ifndef SDPA_DAEMON_JOB_MANAGER_HPP
#define SDPA_DAEMON_JOB_MANAGER_HPP 1

#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/exceptions.hpp>

namespace sdpa { namespace tests { class DaemonFSMTest_SMC; class DaemonFSMTest_BSC;}}

namespace sdpa { namespace daemon {
  class JobManager  {
  public:
	  JobManager();
	  virtual ~JobManager();
	  virtual Job::ptr_t findJob(const sdpa::job_id_t& ) throw(JobNotFoundException) ;
	  virtual void addJob(const sdpa::job_id_t&, const Job::ptr_t& ) throw(JobNotAddedException) ;
	  virtual void deleteJob(const sdpa::job_id_t& ) throw(JobNotDeletedException) ;
	  void markJobForDeletion(const sdpa::job_id_t& job_id, const Job::ptr_t& pJob) throw(JobNotMarkedException);
	  std::vector<sdpa::job_id_t> getJobIDList();

	  //only for testing purposes!
	  friend class sdpa::tests::DaemonFSMTest_SMC;
	  friend class sdpa::tests::DaemonFSMTest_BSC;

  protected:
	  SDPA_DECLARE_LOGGER();
	  typedef std::map<sdpa::job_id_t, Job::ptr_t> job_map_t;

	  job_map_t job_map_;
	  job_map_t job_map_marked_for_del_;
  };
}}

#endif
