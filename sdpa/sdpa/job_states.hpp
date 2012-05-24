#ifndef SDPA_JOB_STATES_HPP
#define SDPA_JOB_STATES_HPP

#include <string>
#include <map>

namespace sdpa
{
  struct status
  {
    enum code
      {
        UNKNOWN

      // non-terminal states
      , PENDING
      , SUSPENDED
      , RUNNING

      // terminal states
      , FINISHED
      , FAILED
      , CANCELED
      };

    typedef std::map<std::string, int> job_state_map_t;

    static job_state_map_t job_state_map()
    {
      job_state_map_t m;

      m["SDPA::Unknown"]   = UNKNOWN;
      m["SDPA::Pending"]   = PENDING;
      m["SDPA::Suspended"] = SUSPENDED;
      m["SDPA::Running"]   = RUNNING;
      m["SDPA::Finished"]  = FINISHED;
      m["SDPA::Failed"]    = FAILED;
      m["SDPA::Canceled"]  = CANCELED;

      return m;
    }

    static std::string show(int code)
    {
      switch (code)
      {
      case PENDING:
        return "SDPA::Pending";
      case SUSPENDED:
        return "SDPA::Suspended";
      case RUNNING:
        return "SDPA::Running";
      case FINISHED:
        return "SDPA::Finished";
      case FAILED:
        return "SDPA::Failed";
      case CANCELED:
        return "SDPA::Canceled";
      case UNKNOWN:
        return "SDPA::Unknown";
      default:
        return "Strange job state";
      }
    }

    static int read(std::string const &state)
    {
      static const job_state_map_t m(job_state_map());
      job_state_map_t::const_iterator it;

      it = m.find(state);
      if (it == m.end()) return UNKNOWN;
      else               return it->second;
    }
  };
}

#endif
