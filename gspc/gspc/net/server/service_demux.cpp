#include <errno.h>

#include <boost/bind.hpp>

#include <gspc/net/frame.hpp>
#include <gspc/net/frame_builder.hpp>
#include <gspc/net/error.hpp>
#include <gspc/net/user.hpp>

#include <fhg/util/starts_with.hpp>

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

      service_demux_t::service_demux_t ()
      {
        this->handle ( "/service/help"
                     , boost::bind ( &service_demux_t::do_service_help
                                   , this
                                   , _1, _2, _3
                                   )
                     );
      }

      int service_demux_t::handle ( std::string const & dst
                                  , gspc::net::service::handler_t h
                                  )
      {
        unique_lock lock (m_mutex);

        std::string mangled_dst = dst;
        trim_r (mangled_dst, SERVICE_SEPARATOR);

        m_handler_map [mangled_dst] = h;

        //! \todo RV do not return const 0 and set return type to void
        return 0;
      }

      int service_demux_t::unhandle (std::string const & dst)
      {
        unique_lock lock (m_mutex);

        std::string mangled_dst = dst;
        trim_r (mangled_dst, SERVICE_SEPARATOR);

        handler_map_t::iterator it = m_handler_map.find (mangled_dst);
        if (it != m_handler_map.end ())
        {
          m_handler_map.erase (it);
          return 0;
        }

        return E_SERVICE_LOOKUP;
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

          if (fhg::util::starts_with (service, dst))
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
                                           , dst
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
                                         , dst
                                         );
          rply.add_body (" failed: ");
          rply.add_body (ex.what ());

          user->deliver (rply);

          return E_SERVICE_FAILED;
        }

        return 0;
      }

      void service_demux_t::do_service_help ( std::string const &
                                            , frame const &rqst
                                            , user_ptr user
                                            )
      {
        frame rply = make::reply_frame (rqst);

        handler_map_t::const_iterator it = m_handler_map.begin ();
        const handler_map_t::const_iterator end = m_handler_map.end ();

        while (it != end)
        {
          rply.add_body (it->first);
          rply.add_body ("\n");
          ++it;
        }

        //! \todo explain why the return value can be ignored
        user->deliver (rply);
      }

      scoped_service_handler::scoped_service_handler
          ( std::string name
          , gspc::net::service::handler_t function
          , gspc::net::server::service_demux_t& service_demux
          )
        : _name (name)
        , _service_demux (service_demux)
      {
        _service_demux.handle (_name, function);
      }
      scoped_service_handler::~scoped_service_handler()
      {
        _service_demux.unhandle (_name);
      }
    }
  }
}
