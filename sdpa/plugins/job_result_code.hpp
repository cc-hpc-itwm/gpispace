#ifndef JOB_RESULT_CODE_HPP
#define JOB_RESULT_CODE_HPP

namespace fhg
{
  namespace wfe
  {
    enum job_result_code_t
      {
        NO_FAILURE                = 0

      , INVALID_JOB_DESCRIPTION   = 1

      , MODULE_CALL_FAILED        = 10
      , MODULE_CALL_SEGFAULTED    = 11
      , MODULE_CALL_ABORTED       = 12

      , JOB_CANCELLED             = 30
      , WALLTIME_EXCEEDED         = 31
      , RESOURCE_LIMIT_REACHED    = 32

      , MODULE_NOT_FOUND          = 50
      , MODULE_LOAD_FAILED        = 51
      , MODULE_UNDEFINED_FUNCTION = 52

      , UNEXPECTED_ERROR          = 100
      };

    inline
    const char *job_result_code_to_string (const int code)
    {
      switch (code)
      {
      case NO_FAILURE:
        return "no failure";

      case INVALID_JOB_DESCRIPTION:
        return "invalid job description";

      case MODULE_CALL_FAILED:
        return "module call failed";
      case MODULE_CALL_SEGFAULTED:
        return "module call segfaulted";
      case MODULE_CALL_ABORTED:
        return "module call aborted";
      case JOB_CANCELLED:
        return "job cancelled";
      case WALLTIME_EXCEEDED:
        return "walltime exceeded";
      case RESOURCE_LIMIT_REACHED:
        return "resource limit reached";
      case MODULE_NOT_FOUND:
        return "module not found";
      case MODULE_LOAD_FAILED:
        return "module load failed";
      case MODULE_UNDEFINED_FUNCTION:
        return "function not defined in module";
      case UNEXPECTED_ERROR:
        return "unexpected error";
      default:
        return "unknown error";
      }
    }
  }
}

#endif
