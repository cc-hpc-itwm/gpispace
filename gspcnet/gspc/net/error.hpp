#ifndef GSPC_NET_ERROR_HPP
#define GSPC_NET_ERROR_HPP

namespace gspc
{
  namespace net
  {
    enum error_code_t
      {
        E_OK                      = 200

      , E_UNAUTHORIZED            = 401
      , E_SERVICE_LOOKUP          = 404
      , E_COMMAND_TOO_LONG        = 414
      , E_HEADER_FIELDS_TOO_LARGE = 431

      , E_INTERNAL_ERROR          = 500
      , E_NOT_IMPLEMENTED         = 501
      , E_SERVICE_FAILED          = 503
      , E_VERSION_NOT_SUPPORTED   = 505
      , E_PERMISSION_DENIED       = 550
      };

    const char * error_string (const error_code_t);
  }
}

#endif
