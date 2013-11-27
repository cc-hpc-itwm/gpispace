#ifndef SDPA_DAEMON_JOB_MANAGER_HPP
#define SDPA_DAEMON_JOB_MANAGER_HPP

#include <sdpa/daemon/Job.hpp>
#include <sdpa/engine/IWorkflowEngine.hpp>

#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

namespace sdpa
{
  namespace daemon
  {
    class GenericDaemon;
    class JobManager
    {
    public:
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;
      typedef boost::unordered_map<sdpa::job_id_t, job_requirements_t>
        requirements_map_t;
      typedef boost::unordered_map<sdpa::job_id_t, sdpa::daemon::Job*>
        job_map_t;
      typedef job_map_t::iterator iterator;

      Job* findJob (const sdpa::job_id_t&) const;
      void addJob ( const sdpa::job_id_t&
                  , Job*
                  , const job_requirements_t& = job_requirements_t()
                  );
      void deleteJob (const sdpa::job_id_t&);

      void addJobRequirements (const sdpa::job_id_t&, const job_requirements_t&);
      const job_requirements_t getJobRequirements (const sdpa::job_id_t&) const;

      bool hasJobs() const;

      void resubmitResults (GenericDaemon*) const;

  protected:
      mutable mutex_type _job_map_and_requirements_mutex;
      job_map_t job_map_;
      requirements_map_t job_requirements_;
    };
  }
}

#endif
