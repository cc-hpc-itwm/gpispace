#ifndef FHG_ERROR_CODES_HPP
#define FHG_ERROR_CODES_HPP

namespace fhg
{
  namespace error
  {
    enum code_t
      {
        NO_ERROR                = 0

      , INVALID_JOB_DESCRIPTION = 1

      , MODULE_CALL_FAILED      = 10
      , MODULE_CALL_SEGFAULTED  = 11
      , MODULE_CALL_ABORTED     = 12

      , EXECUTION_CANCELED     = 30
      , WALLTIME_EXCEEDED       = 31
      , RESOURCE_LIMIT_EXCEEDED = 32

      , MODULE_NOT_FOUND        = 50
      , MODULE_LOAD_FAILED      = 51
      , MODULE_NO_SUCH_FUNCTION = 52

      , PERMISSION_DENIED       = 100
      , OPERATION_NOT_ALLOWED   = 101
      , NOT_OWNER               = 102

      , WORKER_TIMEDOUT         = 200
      , WORKER_UNAVAILABLE      = 201
      , CAPABILITY_MISMATCH     = 202

      , SDPA_NOT_STARTABLE      = 300
      , GPI_UNAVAILABLE         = 301

      , UNEXPECTED_ERROR        = 666
      , UNASSIGNED_ERROR        = 667
      };

    inline
    const char *show(const int code)
    {
      switch (code)
      {
      case NO_ERROR:
        return "no failure";

      case INVALID_JOB_DESCRIPTION:
        return "invalid job description";

      case MODULE_CALL_FAILED:
        return "module call failed";
      case MODULE_CALL_SEGFAULTED:
        return "module call segfaulted";
      case MODULE_CALL_ABORTED:
        return "module call aborted";

      case EXECUTION_CANCELED:
        return "execution canceled";
      case WALLTIME_EXCEEDED:
        return "walltime exceeded";
      case RESOURCE_LIMIT_EXCEEDED:
        return "resource limit exceeded";

      case MODULE_NOT_FOUND:
        return "module not found";
      case MODULE_LOAD_FAILED:
        return "module load failed";
      case MODULE_NO_SUCH_FUNCTION:
        return "function not defined in module";

      case PERMISSION_DENIED:
        return "permission denied";
      case OPERATION_NOT_ALLOWED:
        return "operation not allowed";
      case NOT_OWNER:
        return "not owner";

      case WORKER_TIMEDOUT:
        return "worker timed out";
      case WORKER_UNAVAILABLE:
        return "no worker available";
      case CAPABILITY_MISMATCH:
        return "a required capability was not met";

      case SDPA_NOT_STARTABLE:
        return "sdpa could not be started";
      case GPI_UNAVAILABLE:
        return "gpi is unavailable";

      case UNEXPECTED_ERROR:
        return "unexpected error";
      default:
        return "unassigned error (error code not yet assigned)";
      }
    }
  }
}

#endif
