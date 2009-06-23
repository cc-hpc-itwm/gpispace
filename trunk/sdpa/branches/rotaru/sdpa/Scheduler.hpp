#ifndef SDPA_SCHEDULER_HPP
#define SDPA_SCHEDULER_HPP 1

#include <sdpa/Job.hpp>

namespace sdpa {
  class Scheduler {
  public:
    virtual void acknowledge(const sdpa::Job::job_id_t job_id &) = 0;
    virtual sdpa::Job::ptr_t get_next_job(const Node::worker_id_t &worker_id, const sdpa::Job::job_id_t &last_job) = 0;
    virtual void schedule_local(const sdpa::Job::ptr_t &) = 0;
    virtual void schedule(const sdpa::Job::ptr_t &) = 0;
  };
}

#endif
