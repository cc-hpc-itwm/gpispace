#ifndef SDPA_DAEMON_WORKER_MANAGER_HPP
#define SDPA_DAEMON_WORKER_MANAGER_HPP 1

#include <sdpa/daemon/Worker.hpp>
#include <sdpa/daemon/exceptions.hpp>

namespace sdpa { namespace tests { class DaemonFSMTest_SMC; class DaemonFSMTest_BSC;}}

namespace sdpa { namespace daemon {
  class WorkerManager  {
  public:
	  WorkerManager();
	  virtual ~WorkerManager();

	  Worker::ptr_t findWorker(const Worker::worker_id_t& worker_id) throw(WorkerNotFoundException);

	  //only for testing purposes!
	  friend class sdpa::tests::DaemonFSMTest_SMC;
	  friend class sdpa::tests::DaemonFSMTest_BSC;

  protected:
	  SDPA_DECLARE_LOGGER();
	  typedef std::map<Worker::worker_id_t, Worker::ptr_t> worker_map_t;

	  worker_map_t worker_map_;
  };
}}

#endif
