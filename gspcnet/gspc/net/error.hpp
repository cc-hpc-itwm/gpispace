#ifndef GSPC_NET_ERROR_HPP
#define GSPC_NET_ERROR_HPP

namespace gspc
{
  namespace net
  {
    enum error_code_t
      {
        E_SERVICE_LOOKUP = 404
      , E_INTERNAL_ERROR = 500
      , E_SERVICE_FAILED = 503
      };
  }
}

#endif
