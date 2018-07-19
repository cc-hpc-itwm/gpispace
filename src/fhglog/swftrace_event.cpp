#include <fhglog/swftrace_event.hpp>

#include <fhg/util/starts_with.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>

#include <iomanip>

namespace fhg
{
  namespace log
  {
    namespace
    {
      int const DEFAULT_ALLOC_PROCS = 1;
      double const DEFAULT_USED_CPU_TIME_S = -1;
      double const DEFAULT_USED_MEM_KB = -1;
      int const DEFAULT_REQ_PROCS = -1;
      double const DEFAULT_REQ_TIME_S = -1;
      double const DEFAULT_REQ_MEM_KB = -1;
      int const DEFAULT_USER_ID = 1;
      int const DEFAULT_GROUP_ID = 1;
      int const DEFAULT_QUEUE = 1;
      int const DEFAULT_PREC_JOB_ID = -1;
      double const DEFAULT_TIME_AFTER_PREC_JOB_S = -1;

      std::string const& DELIM()
      {
        static std::string const _ (" ");
        return _;
      }

      enum swftrace_state_t
      {
        SWFTRACE_STATE_FAILED = 0,
        SWFTRACE_STATE_FINISHED,
        SWFTRACE_STATE_CANCELED,
      };

      enum swftrace_job_type_t
      {
        SWFTRACE_JOB_TYPE_CPU_COMPUTE = 1,
        SWFTRACE_JOB_TYPE_GPU_COMPUTE,
        SWFTRACE_JOB_TYPE_IO_GET,
        SWFTRACE_JOB_TYPE_IO_PUT,
        SWFTRACE_JOB_TYPE_OTHER,
      };
    }

    int SWFTraceEvent::get_state (const int state)
    {
      switch (state)
      {
      case sdpa::daemon::NotificationEvent::STATE_FINISHED:
        return SWFTRACE_STATE_FINISHED;
      case sdpa::daemon::NotificationEvent::STATE_VMEM_PUT_FINISHED:
        return SWFTRACE_STATE_FINISHED;
      case sdpa::daemon::NotificationEvent::STATE_VMEM_GET_FINISHED:
        return SWFTRACE_STATE_FINISHED;
      case sdpa::daemon::NotificationEvent::STATE_CANCELED:
        return SWFTRACE_STATE_CANCELED;
      case sdpa::daemon::NotificationEvent::STATE_FAILED:
        return SWFTRACE_STATE_FAILED;

      default:
        throw std::logic_error ("undefined job state");
      }
    }

    unsigned int SWFTraceEvent::get_job_type_id (std::string activity_id)
    {
      if (fhg::util::ends_with ("exec", activity_id))
      {
        return SWFTRACE_JOB_TYPE_CPU_COMPUTE;
      }
      else if (fhg::util::ends_with ("get", activity_id))
      {
        return SWFTRACE_JOB_TYPE_IO_GET;
      }
      else if (fhg::util::ends_with ("put", activity_id))
      {
        return SWFTRACE_JOB_TYPE_IO_PUT;
      }
      return SWFTRACE_JOB_TYPE_OTHER;
    }


    SWFTraceEvent::SWFTraceEvent ( unsigned int job_id
                                 , sec_duration submit_timestamp
                                 , sec_duration start_timestamp
                                 , sec_duration end_timestamp
                                 , int status
                                 , unsigned int job_type_id
                                 , unsigned int partition
                                 )
      : job_id (job_id)
      , submit_time_s (submit_timestamp.count())
      , wait_time_s (sec_duration (start_timestamp - submit_timestamp).count())
      , run_time_s (sec_duration (end_timestamp - start_timestamp).count())
      , n_alloc_procs (DEFAULT_ALLOC_PROCS)
      , used_cpu_time_s (DEFAULT_USED_CPU_TIME_S)
      , used_memory_kb (DEFAULT_USED_MEM_KB)
      , n_req_procs (DEFAULT_REQ_PROCS)
      , req_time_s (DEFAULT_REQ_TIME_S)
      , req_memory_kb (DEFAULT_REQ_MEM_KB)
      , status (status)
      , user_id (DEFAULT_USER_ID)
      , group_id (DEFAULT_GROUP_ID)
      , job_type_id (job_type_id)
      , queue (DEFAULT_QUEUE)
      , partition (partition)
      , prec_job_id (DEFAULT_PREC_JOB_ID)
      , time_after_prec_job_s (DEFAULT_TIME_AFTER_PREC_JOB_S)
    {}

    namespace encode
    {
      namespace
      {
        class tstamp
        {
        public:
          explicit tstamp (double const& t)
            : _t (t)
          {}

          friend std::ostream& operator<< (std::ostream& os, tstamp const& t)
          {
            std::ios_base::fmtflags flags (os.flags());
            os.unsetf (std::ios_base::scientific);
            os.setf (std::ios_base::fixed);

            os << std::setprecision (3) << t._t;

            os.flags (flags);

            return os;
          }

        private:
          double const& _t;
        };
      }
    }

    std::ostream& operator<<
      (std::ostream& os, const fhg::log::SWFTraceEvent& event)
    {
      return os << event.job_id
                << DELIM()
                << encode::tstamp (event.submit_time_s)
                << DELIM()
                << encode::tstamp (event.wait_time_s)
                << DELIM()
                << encode::tstamp (event.run_time_s)
                << DELIM()
                << event.n_alloc_procs
                << DELIM()
                << event.used_cpu_time_s
                << DELIM()
                << event.used_memory_kb
                << DELIM()
                << event.n_req_procs
                << DELIM()
                << event.req_time_s
                << DELIM()
                << event.req_memory_kb
                << DELIM()
                << event.status
                << DELIM()
                << event.user_id
                << DELIM()
                << event.group_id
                << DELIM()
                << event.job_type_id
                << DELIM()
                << event.queue
                << DELIM()
                << event.partition
                << DELIM()
                << event.prec_job_id
                << DELIM()
                << event.time_after_prec_job_s;
    }
  }
}
