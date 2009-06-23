#ifndef SDPA_DAEMON_GENERIC_DAEMON_HPP
#define SDPA_DAEMON_GENERIC_DAEMON_HPP 1

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/exception-translator.hpp>

#include <seda/Strategy.hpp>

#include <sdpa/Job.hpp>
#include <sdpa/Scheduler.hpp>


namespace sdpa { namespace daemon {
  class GenericDaemon : public seda::Strategy {
  public:
    GenericDaemon(const std::string &name, const std::string &outputStage);
    virtual ~GenericDaemon();

    virtual void perform(const seda::IEvent::Ptr &);

    virtual void onStageStart(const std::string &stageName);
    virtual void onStageStop(const std::string &stageName);

  protected:
    // FIXME: implement as a standalone class
    typedef std::map<sdpa::Job::job_id_t, sdpa::Job::ptr_t> job_map_t;
    job_map_t job_map_;
    Scheduler::ptr_t scheduler_;
  };

  /*
  class Orchestrator : public GenericDaemon {

  };
  */
}}

#endif
