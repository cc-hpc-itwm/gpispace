#pragma once

#include <chrono>
#include <ostream>

namespace fhg
{
  namespace log
  {
    class SWFTraceEvent
    {
    public:
      using sec_duration = std::chrono::duration<double, std::chrono::seconds::period>;

      SWFTraceEvent ( unsigned int job_id
                    , sec_duration submit_timestamp
                    , sec_duration start_timestamp
                    , sec_duration end_timestamp
                    , int status
                    , unsigned int job_type_id
                    , unsigned int partition
                    );

      friend std::ostream& operator<< (std::ostream&, SWFTraceEvent const&);

      static int get_state (const int state);
      static unsigned int get_job_type_id (std::string activity_id);

    private:
      const unsigned int job_id;
      const double submit_time_s;            // time in seconds (since the start of the tracing procedure) when the job was submitted to the system
      const double wait_time_s;              // time in seconds between submission and the time at which the job actually began to run
      const double run_time_s;
      const int n_alloc_procs;
      const double used_cpu_time_s;
      const double used_memory_kb;
      const int n_req_procs;                  // requested Number of Processors
      const double req_time_s;                // requested Time (e.g., user runtime estimate or upper bound used in backfilling)
      const double req_memory_kb;             // requested Memory
      const int status;
      const unsigned int user_id;
      const unsigned int group_id;
      const unsigned int job_type_id;         // natural number, between one and the number of different applications appearing in the workload
      const unsigned int queue;
      const unsigned int partition;           // number to identify the different partitions in the systems (can be used to identify the machine on which the job was executed)
      const int prec_job_id;                  // previous job in the workload (current job can only start after its termination)
      const double time_after_prec_job_s;
    };
  }
}
