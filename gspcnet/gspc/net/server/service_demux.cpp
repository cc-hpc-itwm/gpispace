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
          return -ESRCH;
        }

        try
        {
          it->second (dst, rqst, rply);
        }
        catch (std::exception const &ex)
        {
          rply = make::error_frame (E_INTERNAL_ERROR, ex.what ());
          rply.set_body (rqst.to_string ());
          return -EFAULT;
        }

        return 0;
      }
    }
  }
}
