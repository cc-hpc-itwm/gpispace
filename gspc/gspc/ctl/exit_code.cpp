#include "exit_code.hpp"

#include <cerrno>
#include <sysexits.h>
#include <cstring>

#include <boost/format.hpp>

namespace gspc
{
  namespace ctl
  {
    std::string strerror (int ec)
    {
      if (ec < 0)
        return ::strerror (-ec);

      switch (ec)
      {
      case EXIT_SUCCESS:
        return "success";
      case EXIT_FAILURE:
        return "failed";
      case EX_USAGE:            // 64
        return "command line usage error";
      case EX_DATAERR:          // 65
        return "data format error";
      case EX_NOINPUT:          // 66
        return "cannot open input";
      case EX_NOUSER:           // 67
        return "addressee unknown";
      case EX_NOHOST:           // 68
        return "host name unknown";
      case EX_UNAVAILABLE:      // 69
        return "service unavailable";
      case EX_SOFTWARE:         // 70
        return "internal software error";
      case EX_OSERR:            // 71
        return "system error (e.g., can't fork)";
      case EX_OSFILE:           // 72
        return "critical OS file missing";
      case EX_CANTCREAT:        // 73
        return "can't create (user) output file";
      case EX_IOERR:            // 74
        return "input/output error";
      case EX_TEMPFAIL:         // 75
        return "temp failure; user is invited to retry";
      case EX_PROTOCOL:         // 76
        return "remote error in protocol";
      case EX_NOPERM:           // 77
        return "permission denied";
      case EX_CONFIG:           // 78
        return "configuration error";
      default:
        return (boost::format ("unknown exit-code: %1%") % ec).str ();
      }
    }
  }
}
