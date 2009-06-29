#ifndef SDPA_DAEMON_GENERIC_DAEMON_ACTIONS
#define SDPA_DAEMON_GENERIC_DAEMON_ACTIONS 1

namespace sdpa { namespace daemon {
  class GenericDaemonActions {
  public:
    virtual void action_submitJob(/*...*/) = 0;
  };
}}

#endif
