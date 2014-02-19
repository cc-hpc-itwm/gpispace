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
      , CANCELING
      , UNKNOWN
      };

    inline bool is_terminal (code c)
    {
      return c == FINISHED || c == FAILED || c == CANCELED;
    }

    inline bool is_canceled (code c)
    {
      return c == CANCELED;
    }

    inline bool is_pending (code c)
    {
      return c == PENDING;
    }

    inline bool is_canceling (code c)
    {
      return c == CANCELING;
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
      default:
        return "SDPA::Unknown";
      }
    }
  };
}

#endif
