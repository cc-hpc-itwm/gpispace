#include "error.hpp"
#include <gspc/net/frame.hpp>
#include <fhg/util/read.hpp>

namespace boost
{
  namespace system
  {
    template <> struct is_error_code_enum<gspc::net::error_code_t>
    {
      static const bool value = true;
    };
  }
}

namespace gspc
{
  namespace net
  {
    namespace detail
    {
      class error_category : public boost::system::error_category
      {
      public:
        const char *name () const
        {
          return "gspc.net";
        }

        std::string message (int value) const
        {
          switch (value)
          {
          case E_OK:                      // 200
            return "success";
          case E_BAD_REQUEST:             // 400
            return "bad request";
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
      };

      static const boost::system::error_category & get_gspc_net_category ()
      {
        static detail::error_category cat;
        return cat;
      }
    }

    std::string error_string (const error_code_t code)
    {
      return detail::get_gspc_net_category ().message (code);
    }

    boost::system::error_code make_error_code (error_code_t e)
    {
      return boost::system::error_code ( static_cast<int>(e)
                                       , detail::get_gspc_net_category ()
                                       );
    }

    boost::system::error_code make_error_code (frame const &frame)
    {
      if (frame.get_command () == "ERROR" && frame.get_header ("code"))
      {
        error_code_t e =
          static_cast<error_code_t>(fhg::util::read<int>(*frame.get_header ("code")));
        return make_error_code (e);
      }
      else
      {
        return boost::system::errc::make_error_code
          (boost::system::errc::bad_message);
      }
    }
  }
}
