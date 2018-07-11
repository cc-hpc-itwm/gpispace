
#include <fhglog/swftrace_event.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>

#include <iostream>
#include <sstream>


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
        os << _t;
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

    static const int DEFAULT_ALLOC_PROCS = -1;
    static const double DEFAULT_USED_CPU_TIME_S = -1;
    static const double DEFAULT_USED_MEM_KB = -1;
    static const int DEFAULT_REQ_PROCS = -1;
    static const double DEFAULT_REQ_TIME_S = -1;
    static const double DEFAULT_REQ_MEM_KB = -1;
    static const int DEFAULT_USER_ID = 1;
    static const int DEFAULT_GROUP_ID = 1;
    static const int DEFAULT_JOB_TYPE_ID = 1;
    static const int DEFAULT_QUEUE = 1;
    static const int PREC_JOB_ID = -1;
    static const double DEFAULT_TIME_AFTER_PREC_JOB_S = -1;

    static const std::string DELIM = " ";

    enum swftrace_state_t
          {
           SWFTRACE_STATE_FAILED = 0
          , SWFTRACE_STATE_FINISHED
          , SWFTRACE_STATE_PARTIAL_EXEC_CONT
          , SWFTRACE_STATE_PARTIAL_EXEC_FINISHED
          , SWFTRACE_STATE_PARTIAL_EXEC_FAILED
          , SWFTRACE_STATE_CANCELED
          };

    int SWFTraceEvent::get_state(const int state) {
      switch(state) {
      case sdpa::daemon::NotificationEvent::STATE_FINISHED:
      case sdpa::daemon::NotificationEvent::STATE_HACK_WAS_PUT:
      case sdpa::daemon::NotificationEvent::STATE_HACK_WAS_GET:
         return SWFTRACE_STATE_FINISHED;
      case sdpa::daemon::NotificationEvent::STATE_CANCELED:
        return SWFTRACE_STATE_CANCELED;
      case sdpa::daemon::NotificationEvent::STATE_FAILED:
        return SWFTRACE_STATE_FAILED;

      case sdpa::daemon::NotificationEvent::STATE_STARTED:
      default: // we should never get here
        // TODO throw exception
        return SWFTRACE_STATE_FAILED;
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
        unsigned int group_id,
        unsigned int job_type_id,
        unsigned int queue,
        unsigned int partition,
        int prec_job_id,
        double time_after_prec_job_s,
        double trace_start_timestamp
        ) :
    job_id(job_id), submit_time_s(submit_timestamp - trace_start_timestamp),
    wait_time_s(start_timestamp - submit_timestamp), run_time_s(end_timestamp - start_timestamp),
    n_alloc_procs(n_alloc_procs), used_cpu_time_s(used_cpu_time_s), used_memory_kb(used_memory_kb),
    n_req_procs(n_req_procs), req_time_s(req_time_s), req_memory_kb(req_memory_kb),
    status(status),
    user_id(user_id), group_id(group_id),
    job_type_id(job_type_id), queue(queue), partition(partition),
    prec_job_id(prec_job_id), time_after_prec_job_s(time_after_prec_job_s)
    {}

    SWFTraceEvent::SWFTraceEvent ( unsigned int job_id,
              double submit_timestamp,
              double start_timestamp,
              double end_timestamp,
              int status,
              unsigned int partition,
              double trace_start_timestamp):
    SWFTraceEvent(job_id, submit_timestamp, start_timestamp, end_timestamp,
        DEFAULT_ALLOC_PROCS, DEFAULT_USED_CPU_TIME_S, DEFAULT_USED_MEM_KB,
        DEFAULT_REQ_PROCS, DEFAULT_REQ_TIME_S, DEFAULT_REQ_MEM_KB,
        status,
        DEFAULT_USER_ID, DEFAULT_GROUP_ID, DEFAULT_JOB_TYPE_ID, DEFAULT_QUEUE,
        partition,
        PREC_JOB_ID, DEFAULT_TIME_AFTER_PREC_JOB_S,
        trace_start_timestamp)
    {}

    SWFTraceEvent::SWFTraceEvent ( unsigned int job_id,
              double submit_timestamp,
              double start_timestamp,
              double end_timestamp,
              int status,
              unsigned int partition):
    SWFTraceEvent(job_id, submit_timestamp, start_timestamp, end_timestamp,
            status, partition, submit_timestamp)
    {}



    std::string SWFTraceEvent::encoded() const {
      std::ostringstream os;
      os << job_id << DELIM << encode::tstamp(submit_time_s) << DELIM << encode::tstamp(wait_time_s) << DELIM;
      os << encode::tstamp(run_time_s) << DELIM << n_alloc_procs << DELIM << used_cpu_time_s << DELIM << used_memory_kb << DELIM;
      os << n_req_procs << DELIM << req_time_s << DELIM << req_memory_kb << DELIM << status << DELIM;
      os << user_id << DELIM << group_id << DELIM << job_type_id << DELIM;
      os << queue << DELIM << partition << DELIM << prec_job_id << DELIM << time_after_prec_job_s;
      return os.str();
    }

  }
}



std::ostream& operator<< (std::ostream& os, const fhg::log::SWFTraceEvent& event)
{
  os << event.encoded() << std::endl;
  return os;
}





