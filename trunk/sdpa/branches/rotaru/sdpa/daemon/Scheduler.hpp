#ifndef SDPA_SCHEDULER_HPP
#define SDPA_SCHEDULER_HPP 1

#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/Worker.hpp>
#include <sdpa/daemon/exceptions.hpp>

namespace sdpa {
namespace daemon {
  class Scheduler {
  public:

	 typedef sdpa::shared_ptr<Scheduler> ptr_t;

	 virtual Worker::ptr_t& findWorker(const Worker::worker_id_t&  ) throw(WorkerNotFoundException) = 0;
	 virtual void addWorker(const  Worker::ptr_t& ) = 0;
	 virtual int  numberOfWorkers() = 0;
	 virtual void schedule(Job::ptr_t& pJob) = 0;
	 virtual void start()=0;
	 virtual void stop()=0;
	 virtual void run()=0;
  };
}}

#endif
