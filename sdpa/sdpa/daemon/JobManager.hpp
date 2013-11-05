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
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#include <sdpa/engine/IWorkflowEngine.hpp>

namespace sdpa {
  namespace daemon {
    class JobManager
    {
    public:
	    typedef sdpa::shared_ptr<JobManager> ptr_t;
	    typedef boost::recursive_mutex mutex_type;
	    typedef boost::unique_lock<mutex_type> lock_type;
	    typedef boost::unordered_map<sdpa::job_id_t, job_requirements_t> requirements_map_t;
	    typedef boost::unordered_map<sdpa::job_id_t, sdpa::daemon::Job::ptr_t> job_map_t;
	    typedef job_map_t::iterator iterator;

	    JobManager(const std::string& str="");
	    virtual ~JobManager();

	    Job::ptr_t& findJob(const sdpa::job_id_t& ) throw (JobNotFoundException) ;
	    void addJob(const sdpa::job_id_t&, const Job::ptr_t& ) throw(JobNotAddedException) ;
	    void deleteJob(const sdpa::job_id_t& ) throw(JobNotDeletedException) ;

	    void addJobRequirements( const sdpa::job_id_t&, const job_requirements_t& ) throw (JobNotFoundException);
	    const job_requirements_t getJobRequirements(const sdpa::job_id_t& jobId) const throw (NoJobRequirements);

	    size_t countMasterJobs() const;
	    size_t getNumberOfJobs() const;

	    void waitForFreeSlot();
	    bool slotAvailable() const;

	    void resubmitResults(IAgent* );
	    sdpa::job_id_list_t getListNotCompletedMasterJobs(bool bHasWfe);

	    std::string print() const;

  protected:
	    SDPA_DECLARE_LOGGER();
	    job_map_t job_map_;
	    mutable mutex_type mtx_;
	    boost::condition_variable_any free_slot_;
	    requirements_map_t job_requirements_;
  };
}}

#endif
