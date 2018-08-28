#include <fhglog/swftrace_event.hpp>

#include <iomanip>
#include <sstream>

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
      int const DEFAULT_QUEUE = 1;
      int const DEFAULT_PREC_JOB_ID = -1;
      double const DEFAULT_TIME_AFTER_PREC_JOB_S = -1;

      std::string const& DELIM()
      {
        static std::string const _ (" ");
        return _;
      }

      std::string const& TRACE_COMMENT()
      {
        static std::string const _ ("; ");
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

      std::map<swftrace_state_t, std::string> const& job_states()
      {
        static std::map<swftrace_state_t, std::string> const _
          { {SWFTRACE_STATE_FAILED, "Failed"}
          , {SWFTRACE_STATE_FINISHED, "Finished successfully"}
          , {SWFTRACE_STATE_CANCELED, "Cancelled"}
          };
        return _;
      }

      std::map<swftrace_job_type_t, std::string> const& job_types()
      {
        static std::map<swftrace_job_type_t, std::string> const _
          { {SWFTRACE_JOB_TYPE_CPU_COMPUTE, "CPU Computation task"}
          , {SWFTRACE_JOB_TYPE_GPU_COMPUTE, "GPU Computation task"}
          , {SWFTRACE_JOB_TYPE_IO_GET, "Data transfer task (input data)"}
          , {SWFTRACE_JOB_TYPE_IO_PUT, "Data transfer task (output data)"}
          , {SWFTRACE_JOB_TYPE_OTHER, "Other (e.g., agent)"}
          };
        return _;
      }
    }

    int SWFTraceEvent::get_state (const int state)
    {
      switch (state)
      {
      case sdpa::daemon::NotificationEvent::STATE_FINISHED:
        return SWFTRACE_STATE_FINISHED;
      case sdpa::daemon::NotificationEvent::STATE_CANCELED:
        return SWFTRACE_STATE_CANCELED;
      case sdpa::daemon::NotificationEvent::STATE_FAILED:
        return SWFTRACE_STATE_FAILED;

      default:
        throw std::logic_error ("undefined job state");
      }
    }

    namespace
    {
      swftrace_job_type_t get_job_type_id
        (sdpa::daemon::NotificationEvent::type_t const type)
      {
        switch (type)
        {
        case sdpa::daemon::NotificationEvent::type_t::agent:
          return SWFTRACE_JOB_TYPE_OTHER;
        case sdpa::daemon::NotificationEvent::type_t::module_call:
          return SWFTRACE_JOB_TYPE_CPU_COMPUTE;
        case sdpa::daemon::NotificationEvent::type_t::vmem_get:
          return SWFTRACE_JOB_TYPE_IO_GET;
        case sdpa::daemon::NotificationEvent::type_t::vmem_put:
          return SWFTRACE_JOB_TYPE_IO_PUT;

        default:
          throw std::logic_error ("undefined job type");
        }
      }
    }

    SWFTraceEvent::SWFTraceEvent ( unsigned int job_id
                                 , sec_duration submit_timestamp
                                 , sec_duration start_timestamp
                                 , sec_duration end_timestamp
                                 , int status
                                 , uint64_t group_id
                                 , sdpa::daemon::NotificationEvent::type_t type
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
      , group_id (group_id)
      , job_type_id (get_job_type_id (type))
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

    std::string SWFTraceEvent::gen_swf_trace_header()
    {
      std::ostringstream os;
      os << TRACE_COMMENT() << "Version: 2.2\n"
         << TRACE_COMMENT() << '\n'
         << TRACE_COMMENT() << "Data Fields:\n"
         << TRACE_COMMENT() << " 1. Task number (unique integer id)\n"
         << TRACE_COMMENT() << " 2. Submit Time (in seconds, relative to the time when the monitor was started)\n"
         << TRACE_COMMENT() << " 3. Wait Time (in seconds). The difference between the job's submit time and the time at which it actually began to run.\n"
         << TRACE_COMMENT() << " 4. Run Time (in seconds). The wall clock time the job was running (end time minus start time)\n"
         << TRACE_COMMENT() << " 5. Number of Allocated Processors - always equals 1 (each task is allocated to one worker)\n"
         << TRACE_COMMENT() << " 6. Average CPU Time Used (not used, set to " << DEFAULT_USED_CPU_TIME_S << ")\n"
         << TRACE_COMMENT() << " 7. Used Memory (not used, set to " << DEFAULT_USED_MEM_KB << ")\n"
         << TRACE_COMMENT() << " 8. Requested Number of Processors (not used, set to " << DEFAULT_REQ_PROCS << ")\n"
         << TRACE_COMMENT() << " 9. Requested Time (not used, set to " << DEFAULT_REQ_TIME_S << ")\n"
         << TRACE_COMMENT() << "10. Requested Memory (not used, set to " << DEFAULT_REQ_MEM_KB << ")\n"
         << TRACE_COMMENT() << "11. Status: \n";
      for (auto const& val : job_states())
      {
        os << TRACE_COMMENT() << "    - " << val.first << ": " << val.second << '\n';
      }
      os << TRACE_COMMENT() << "12. User ID (not used, set to " << DEFAULT_USER_ID << ")\n"
         << TRACE_COMMENT() << "13. Group ID (used to group tasks by transition id)\n"
         << TRACE_COMMENT() << "14. Executable Number -- integer value representing the type of task as follows \n";
      for (auto const& val : job_types())
      {
        os << TRACE_COMMENT() << "    - " << val.first << ": " << val.second << '\n';
      }
      os << TRACE_COMMENT() << "15. Queue Number (not used, set to " << DEFAULT_QUEUE << ")\n"
         << TRACE_COMMENT() << "16. Partition Number -- integer value representing the worker id where the task was executed\n"
         << TRACE_COMMENT() << "17. Preceding Job Number (not used, set to " << DEFAULT_PREC_JOB_ID << ")\n"
         << TRACE_COMMENT() << "18. Think Time from Preceding Job (not used, set to " << DEFAULT_TIME_AFTER_PREC_JOB_S << ")\n"
         << TRACE_COMMENT() << '\n'
         << TRACE_COMMENT() << '\n';
      return os.str();
    }
  }
}
