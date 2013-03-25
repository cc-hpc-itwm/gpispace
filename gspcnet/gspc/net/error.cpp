#include "error.hpp"

namespace gspc
{
  namespace net
  {
    const char * error_string (const error_code_t code)
    {
      switch (code)
      {
      case E_OK:                      // 200
        return "success";
      case E_UNAUTHORIZED:            // 401
        return "not authorized";
      case E_SERVICE_LOOKUP:          // 404
        return "no such service";
      case E_COMMAND_TOO_LONG:        // 414
        return "command too long";
      case E_HEADER_FIELDS_TOO_LARGE: // 431
        return "header fields too large";
      case E_INTERNAL_ERROR:          // 500
        return "internal error";
      case E_NOT_IMPLEMENTED:         // 501
        return "not implemented";
      case E_SERVICE_FAILED:          // 503
        return "service failed";
      case E_VERSION_NOT_SUPPORTED:   // 505
        return "version not supported";
      case E_PERMISSION_DENIED:       // 550
        return "permission denied";
      default:
        return "unknown error";
      }
    }
  }
}
