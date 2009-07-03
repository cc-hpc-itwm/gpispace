#ifndef SDPA_DAEMON_GENERIC_DAEMON_HPP
#define SDPA_DAEMON_GENERIC_DAEMON_HPP 1

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/exception-translator.hpp>

#include <seda/Strategy.hpp>

#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/Worker.hpp>
#include <sdpa/daemon/Scheduler.hpp>
#include <sdpa/daemon/GenericDaemonActions.hpp>

namespace sdpa { namespace daemon {
  class GenericDaemon : public seda::Strategy, public sdpa::daemon::GenericDaemonActions {
  public:
    GenericDaemon(const std::string &name, const std::string &outputStage);
    virtual ~GenericDaemon();

    virtual void perform(const seda::IEvent::Ptr &);

    virtual void onStageStart(const std::string &stageName);
    virtual void onStageStop(const std::string &stageName);

  protected:
    // FIXME: implement as a standalone class
    typedef std::map<Job::job_id_t, Job::ptr_t> job_map_t;
    typedef std::map<Worder::worker_id_t, Worker::ptr_t> worker_map_t;
    job_map_t job_map_;
    worker_map_t worker_map_;
    Scheduler::ptr_t scheduler_;
  };

  /*
  class Orchestrator : public GenericDaemon {

  };
  */
}}

#endif
