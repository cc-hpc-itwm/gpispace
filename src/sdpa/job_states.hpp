#pragma once

#include <fhg/util/macros.hpp>

#include <string>

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
      };

    inline bool is_terminal (code c)
    {
      return c == FINISHED || c == FAILED || c == CANCELED;
    }

    inline std::string show (status::code code)
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
      }

      INVALID_ENUM_VALUE (status::code, code);
    }
  };
}
