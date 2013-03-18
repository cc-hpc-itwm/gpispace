#include <errno.h>

#include <gspc/net/frame.hpp>
#include <gspc/net/error.hpp>

#include "service_demux.hpp"

namespace gspc
{
  namespace net
  {
    namespace server
    {
      service_demux_t::service_demux_t ()
      {}

      int service_demux_t::handle ( std::string const & dst
                                  , service_handler_t h
                                  )
      {
        unique_lock lock (m_mutex);
        m_handler_map [dst] = h;
        return 0;
      }

      int service_demux_t::handle_request ( std::string const & dst
                                          , frame const & rqst
                                          , frame & rply
                                          )
      {
        shared_lock lock (m_mutex);
        handler_map_t::iterator it = m_handler_map.find (dst);
        if (it == m_handler_map.end ())
        {
          rply = make::error_frame (E_SERVICE_LOOKUP, "no such service");
          return E_SERVICE_LOOKUP;
        }

        try
        {
          it->second (dst, rqst, rply);
        }
        catch (std::exception const &ex)
        {
          rply = make::error_frame (E_SERVICE_FAILED, "service request failed");
          rply.add_body ("Request to service '" + dst + "' failed:\n");
          rply.add_body (ex.what ());
          return E_SERVICE_FAILED;
        }

        return 0;
      }
    }
  }
}
