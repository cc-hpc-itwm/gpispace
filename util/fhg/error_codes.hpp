#ifndef FHG_ERROR_CODES_HPP
#define FHG_ERROR_CODES_HPP

namespace fhg
{
  namespace error
  {
    enum code_t
      {
        NO_ERROR                = 0

      , EXECUTION_CANCELED     = 30

      , UNKNOWN_ERROR        = 666
      };

    inline
    const char *show(const int code)
    {
      switch (code)
      {
      case NO_ERROR:
        return "no failure";

      case EXECUTION_CANCELED:
        return "execution canceled";

      case UNKNOWN_ERROR:
        return "unknown error";

      default:
        return "unassigned error (error code not yet assigned)";
      }
    }
  }
}

#endif
