#ifndef SDPA_JOB_STATES_HPP
#define SDPA_JOB_STATES_HPP

#include <string>
#include <map>

namespace sdpa
{
  namespace status
  {
    enum code
      {
        // terminal states
        FINISHED
      , FAILED
      , CANCELED

        // non-terminal states
      , PENDING
      , RUNNING
      , STALLED
      , CANCELING
      };

    inline bool is_running (code c)
    {
      return c == RUNNING;
    }

    inline bool is_terminal (code c)
    {
      return c == FINISHED || c == FAILED || c == CANCELED;
    }

    inline std::string show(int code)
    {
      switch (code)
      {
      case PENDING:
        return "SDPA::Pending";
      case RUNNING:
        return "SDPA::Running";
      case FINISHED:
        return "SDPA::Finished";
      case FAILED:
        return "SDPA::Failed";
      case CANCELED:
        return "SDPA::Canceled";
      case CANCELING:
        return "SDPA::Canceling";
      case STALLED:
        return "SDPA::Stalled";
      default:
        return "Strange job state";
      }
    }
  };
}

#endif
