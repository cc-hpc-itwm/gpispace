
#include <fhglog/swftrace_event.hpp>
#include <sdpa/daemon/NotificationEvent.hpp>

#include <fhg/util/starts_with.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>

namespace
{
  namespace encode
  {
    class tstamp
    {
    public:
      explicit
      tstamp (double const &t)
        : _t (t)
      {}
      std::ostream& operator() (std::ostream& os) const
      {
        std::ios_base::fmtflags flags(os.flags());
        os.unsetf (std::ios_base::scientific);
        os.setf (std::ios_base::fixed);
        os << std::setprecision(3) << _t;
        os.flags(flags);

        return os;
      }
    private:
      double const& _t;
    };
    std::ostream& operator<< (std::ostream& os, tstamp const& t)
    {
      return t (os);
    }
  }
}

namespace fhg
{
  namespace log
  {

    static const int DEFAULT_ALLOC_PROCS = 1;
    static const double DEFAULT_USED_CPU_TIME_S = -1;
    static const double DEFAULT_USED_MEM_KB = -1;
    static const int DEFAULT_REQ_PROCS = -1;
    static const double DEFAULT_REQ_TIME_S = -1;
    static const double DEFAULT_REQ_MEM_KB = -1;
    static const int DEFAULT_USER_ID = 1;
    static const int DEFAULT_QUEUE = 1;
    static const int DEFAULT_PREC_JOB_ID = -1;
    static const double DEFAULT_TIME_AFTER_PREC_JOB_S = -1;

    const std::string SWFTraceEvent::get_column_delim()
    {
      static std::string delim = " ";
      return delim;
    }

    const std::string SWFTraceEvent::get_trace_comment()
    {
      static std::string comment = "; ";
      return comment;
    }

    const std::string SWFTraceEvent::get_job_type_name(const swftrace_job_type_t jtype)
    {
      static const std::map<swftrace_job_type_t, std::string> job_types = {
          {SWFTRACE_JOB_TYPE_CPU_COMPUTE, "CPU Computation task"},
          {SWFTRACE_JOB_TYPE_GPU_COMPUTE, "GPU Computation task"},
          {SWFTRACE_JOB_TYPE_IO_GET, "Data transfer task (input data)"},
          {SWFTRACE_JOB_TYPE_IO_PUT, "Data transfer task (output data)"},
          {SWFTRACE_JOB_TYPE_OTHER, "Other (e.g., agent)"}
      };

      if (job_types.count(jtype) <= 0) {
        throw std::invalid_argument ("invalid job type id");
      }

      return job_types.at(jtype);
    }


    const std::string SWFTraceEvent::get_job_state_name(const swftrace_state_t jstate)
    {
      static const std::map<swftrace_state_t, std::string> job_states = {
        {SWFTRACE_STATE_FAILED, "Failed"},
        {SWFTRACE_STATE_FINISHED, "Finished successfully"},
        {SWFTRACE_STATE_CANCELED, "Cancelled"}
      };

      if (job_states.count(jstate) <= 0) {
        throw std::invalid_argument ("invalid job state id");
      }

      return job_states.at(jstate);
    }

    int SWFTraceEvent::get_job_type_id(std::string activity_id)
    {
      if (fhg::util::ends_with ("exec", activity_id))
      {
        return SWFTRACE_JOB_TYPE_CPU_COMPUTE;
      }
      if (fhg::util::ends_with ("get", activity_id))
      {
        return SWFTRACE_JOB_TYPE_IO_GET;
      }
      if (fhg::util::ends_with ("put", activity_id))
      {
        return SWFTRACE_JOB_TYPE_IO_PUT;
      }
      return SWFTRACE_JOB_TYPE_OTHER;
    }

    int SWFTraceEvent::get_state(const int state)
    {
      switch(state)
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

    SWFTraceEvent::SWFTraceEvent ( unsigned int job_id,
        double submit_timestamp,
        double start_timestamp,
        double end_timestamp,
        int n_alloc_procs,
        double used_cpu_time_s,
        double used_memory_kb,
        int n_req_procs,
        double req_time_s,
        double req_memory_kb,
        int status,
        unsigned int user_id,
        uint64_t group_id,
        unsigned int job_type_id,
        unsigned int queue,
        unsigned int partition,
        int prec_job_id,
        double time_after_prec_job_s,
        double trace_start_timestamp
        ):
    job_id(job_id),
    submit_time_s(submit_timestamp - trace_start_timestamp),
    wait_time_s(start_timestamp - submit_timestamp),
    run_time_s(end_timestamp - start_timestamp),
    n_alloc_procs(n_alloc_procs),
    used_cpu_time_s(used_cpu_time_s),
    used_memory_kb(used_memory_kb),
    n_req_procs(n_req_procs),
    req_time_s(req_time_s),
    req_memory_kb(req_memory_kb),
    status(status),
    user_id(user_id),
    group_id(group_id),
    job_type_id(job_type_id),
    queue(queue),
    partition(partition),
    prec_job_id(prec_job_id),
    time_after_prec_job_s(time_after_prec_job_s)
    {}

    SWFTraceEvent::SWFTraceEvent ( unsigned int job_id,
              double submit_timestamp,
              double start_timestamp,
              double end_timestamp,
              int status,
              uint64_t group_id,
              unsigned int job_type_id,
              unsigned int partition,
              double trace_start_timestamp):
    SWFTraceEvent(job_id, submit_timestamp, start_timestamp, end_timestamp,
        DEFAULT_ALLOC_PROCS, DEFAULT_USED_CPU_TIME_S, DEFAULT_USED_MEM_KB,
        DEFAULT_REQ_PROCS, DEFAULT_REQ_TIME_S, DEFAULT_REQ_MEM_KB,
        status,
        DEFAULT_USER_ID,
        group_id,
        job_type_id,
        DEFAULT_QUEUE,
        partition,
        DEFAULT_PREC_JOB_ID, DEFAULT_TIME_AFTER_PREC_JOB_S,
        trace_start_timestamp)
    {}

    SWFTraceEvent::SWFTraceEvent ( unsigned int job_id,
              double submit_timestamp,
              double start_timestamp,
              double end_timestamp,
              int status,
              uint64_t group_id,
              unsigned int job_type_id,
              unsigned int partition):
    SWFTraceEvent(job_id, submit_timestamp, start_timestamp, end_timestamp,
            status, group_id, job_type_id, partition, 0.0)
    {}

    std::string SWFTraceEvent::string() const
    {
      std::ostringstream os;
      const std::string delim = get_column_delim();

      os << job_id << delim << encode::tstamp(submit_time_s) << delim << encode::tstamp(wait_time_s) << delim;
      os << encode::tstamp(run_time_s) << delim << n_alloc_procs << delim << used_cpu_time_s << delim << used_memory_kb << delim;
      os << n_req_procs << delim << req_time_s << delim << req_memory_kb << delim << status << delim;
      os << user_id << delim << group_id << delim << job_type_id << delim;
      os << queue << delim << partition << delim << prec_job_id << delim << time_after_prec_job_s;
      return os.str();
    }

    std::ostream& SWFTraceEvent::operator() (std::ostream& os) const
    {
      return os << string();
    }

    std::string SWFTraceEvent::gen_swf_trace_header()
    {
      std::ostringstream os;
      auto comment = get_trace_comment();
      auto job_states = { SWFTRACE_STATE_FAILED, SWFTRACE_STATE_FINISHED, SWFTRACE_STATE_CANCELED };
      auto job_types = {SWFTRACE_JOB_TYPE_CPU_COMPUTE, SWFTRACE_JOB_TYPE_GPU_COMPUTE,
          SWFTRACE_JOB_TYPE_IO_GET, SWFTRACE_JOB_TYPE_IO_PUT, SWFTRACE_JOB_TYPE_OTHER };


      os << comment << "Version: 2.2" << std::endl;
      os << comment << std::endl;
      os << comment << "Data Fields:" << std::endl;
      os << comment << " 1. Task number (unique integer id)" << std::endl;
      os << comment << " 2. Submit Time (in seconds, relative to the time when the monitor was started)" << std::endl;
      os << comment << " 3. Wait Time (in seconds). The difference between the job's submit time and the time at which it actually began to run." << std::endl;
      os << comment << " 4. Run Time (in seconds). The wall clock time the job was running (end time minus start time)" << std::endl;
      os << comment << " 5. Number of Allocated Processors - always equals 1 (each task is allocated to one worker)" << std::endl;
      os << comment << " 6. Average CPU Time Used (not used, set to " << DEFAULT_USED_CPU_TIME_S << ")" << std::endl;
      os << comment << " 7. Used Memory (not used, set to " << DEFAULT_USED_MEM_KB << ")" << std::endl;
      os << comment << " 8. Requested Number of Processors (not used, set to " << DEFAULT_REQ_PROCS << ")" << std::endl;
      os << comment << " 9. Requested Time (not used, set to " << DEFAULT_REQ_TIME_S << ")" << std::endl;
      os << comment << "10. Requested Memory (not used, set to " << DEFAULT_REQ_MEM_KB << ")" << std::endl;
      os << comment << "11. Status: " << std::endl;
      for (auto val : job_states )
      {
        os << comment << "    - " << val << ": " << get_job_state_name(val) << std::endl;
      }
      os << comment << "12. User ID (not used, set to " << DEFAULT_USER_ID << ")" << std::endl;
      os << comment << "13. Group ID (used to group tasks by transition id)" << std::endl;
      os << comment << "14. Executable Number -- integer value representing the type of task as follows " << std::endl;
      for (auto val : job_types )
      {
        os << comment << "    - " << val << ": " << get_job_type_name(val) << std::endl;
      }
      os << comment << "15. Queue Number (not used, set to " << DEFAULT_QUEUE << ")" << std::endl;
      os << comment << "16. Partition Number -- integer value representing the worker id where the task was executed" << std::endl;
      os << comment << "17. Preceding Job Number (not used, set to " << DEFAULT_PREC_JOB_ID << ")" << std::endl;
      os << comment << "18. Think Time from Preceding Job (not used, set to " << DEFAULT_TIME_AFTER_PREC_JOB_S << ")" << std::endl;
      os << comment << std::endl;
      os << comment << std::endl;
      return os.str();
    }
  }
}



