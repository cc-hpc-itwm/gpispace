#include <errno.h>

#include <gspc/net/frame.hpp>
#include <gspc/net/frame_builder.hpp>
#include <gspc/net/error.hpp>
#include <gspc/net/user.hpp>

#include "service_demux.hpp"

namespace gspc
{
  namespace net
  {
    namespace server
    {
      static const char SERVICE_SEPARATOR = '/';

      static void trim_r (std::string & s, const char c)
      {
        while (s.size () && s [s.size ()-1] == c)
        {
          s.erase (s.end () - 1);
        }
      }

      static bool s_starts_with ( std::string const &s
                                , std::string const &prefix
                                )
      {
        return s.find (prefix) == 0;
      }

      service_demux_t::service_demux_t ()
      {}

      int service_demux_t::handle ( std::string const & dst
                                  , gspc::net::service::handler_t h
                                  )
      {
        unique_lock lock (m_mutex);

        std::string mangled_dst = dst;
        trim_r (mangled_dst, SERVICE_SEPARATOR);

        m_handler_map [mangled_dst] = h;
        return 0;
      }

      template <typename Iterator>
      Iterator find_best_prefix_match ( std::string dst
                                      , Iterator begin
                                      , Iterator end
                                      )
      {
        size_t max_match_length = 0;
        Iterator it = end;

        while (begin != end)
        {
          const std::string & service = begin->first;

          if (s_starts_with (dst, service))
          {
            if (  dst.size () <= service.size ()
               || dst [service.size ()] == SERVICE_SEPARATOR
               )
            {
              if (max_match_length <= service.size ())
              {
                max_match_length = service.size ();
                it = begin;
              }
            }
          }

          ++begin;
        }

        return it;
      }

      int service_demux_t::handle_request ( std::string const & dst
                                          , frame const & rqst
                                          , user_ptr user
                                          )
      {
        shared_lock lock (m_mutex);
        handler_map_t::iterator it =
          find_best_prefix_match ( dst
                                 , m_handler_map.begin ()
                                 , m_handler_map.end ()
                                 );
        if (it == m_handler_map.end ())
        {
          user->deliver (make::error_frame ( rqst
                                           , E_SERVICE_LOOKUP
                                           , "no such service: '" + dst + "'"
                                           )
                        );
          return E_SERVICE_LOOKUP;
        }

        try
        {
          it->second (dst, rqst, user);
        }
        catch (std::exception const &ex)
        {
          frame rply = make::error_frame ( rqst
                                         , E_SERVICE_FAILED
                                         , "service request failed"
                                         );
          rply.add_body ("Request to service '" + dst + "' failed:\n");
          rply.add_body (ex.what ());

          user->deliver (rply);

          return E_SERVICE_FAILED;
        }

        return 0;
      }
    }
  }
}
