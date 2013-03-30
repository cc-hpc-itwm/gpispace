// -*- mode: c++ -*-
#include "base_server.hpp"

#include <fhg/assert.hpp>

#include <boost/foreach.hpp>

#include <gspc/net/server/queue_manager.hpp>
#include <gspc/net/server/url_maker.hpp>
#include <gspc/net/frame_builder.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      template <class Proto>
      base_server<Proto>::base_server ( endpoint_type const &endpoint
                                      , queue_manager_t & qmgr
                                      )
        : m_qmgr (qmgr)
        , m_io_service ()
        , m_acceptor (m_io_service)
        , m_new_connection ()
        , m_thread_pool_size (4u)
        , m_thread_pool ()
      {
        m_acceptor.open (endpoint.protocol());
        m_acceptor.set_option (typename acceptor_type::reuse_address(true));
        m_acceptor.bind (endpoint);
        m_acceptor.listen ();

        start_accept();
      }

      template <class Proto>
      base_server<Proto>::~base_server ()
      {
        stop ();
      }

      template <class Proto>
      int base_server<Proto>::start ()
      {
        assert (m_thread_pool.empty ());

        m_io_service.reset ();

        for (size_t i = 0 ; i < m_thread_pool_size ; ++i)
        {
          thread_ptr_t thrd
            (new boost::thread (boost::bind ( &boost::asio::io_service::run
                                            , &m_io_service
                                            )
                               )
            );
          m_thread_pool.push_back (thrd);
        }

        return 0;
      }

      template <class Proto>
      int base_server<Proto>::stop ()
      {
        m_io_service.stop ();

        BOOST_FOREACH (thread_ptr_t thrd, m_thread_pool)
        {
          thrd->join ();
        }
        m_thread_pool.clear ();

        return 0;
      }

      template <class Proto>
      std::string base_server<Proto>::url () const
      {
        return url_maker<Proto>::make (m_acceptor.local_endpoint ());
      }

      template <class Proto>
      int base_server<Proto>::handle_frame (user_ptr user, frame const &f)
      {
        if      (f.get_command () == "CONNECT")
        {
          return m_qmgr.connect (user, f);
        }
        else if (f.get_command () == "SEND")
        {
          if (not f.has_header ("destination"))
          {
            user->deliver
              (gspc::net::make::error_frame ( gspc::net::E_BAD_REQUEST
                                            , "required header 'destination' is missing"
                                            )
              );
            return -EPROTO;
          }

          return m_qmgr.send (user, *f.get_header ("destination"), f);
        }
        else if (f.get_command () == "REQUEST")
        {
          if (not f.has_header ("destination"))
          {
            user->deliver
              (gspc::net::make::error_frame ( gspc::net::E_BAD_REQUEST
                                            , "required header 'destination' is missing"
                                            )
              );
            return -EPROTO;
          }

          return m_qmgr.request (user, *f.get_header ("destination"), f);
        }
        else if (f.get_command () == "SUBSCRIBE")
        {
          if (not f.has_header ("destination"))
          {
            user->deliver
              (gspc::net::make::error_frame ( gspc::net::E_BAD_REQUEST
                                            , "required header 'destination' is missing"
                                            )
              );
            return -EPROTO;
          }
          if (not f.has_header ("id"))
          {
            user->deliver
              (gspc::net::make::error_frame ( gspc::net::E_BAD_REQUEST
                                            , "required header 'id' is missing"
                                            )
              );
            return -EPROTO;
          }

          return m_qmgr.subscribe ( user
                                  , *f.get_header ("destination")
                                  , *f.get_header ("id")
                                  , f
                                  );
        }
        else if (f.get_command () == "UNSUBSCRIBE")
        {
          if (not f.has_header ("id"))
          {
            user->deliver
              (gspc::net::make::error_frame ( gspc::net::E_BAD_REQUEST
                                            , "required header 'id' is missing"
                                            )
              );
            return -EPROTO;
          }

          return m_qmgr.unsubscribe ( user
                                    , *f.get_header ("id")
                                    , f
                                    );
        }
        else if (f.get_command () == "DISCONNECT")
        {
          return m_qmgr.disconnect (user, f);
        }
        else
        {
          user->deliver
            (gspc::net::make::error_frame ( gspc::net::E_BAD_REQUEST
                                          , "invalid command '" + f.get_command () + "'"
                                          )
            );
          return -EBADRQC;
        }
      }

      template <class Proto>
      int base_server<Proto>::handle_error ( user_ptr user
                                           , boost::system::error_code const &ec
                                           )
      {
        if (ec)
        {
          return m_qmgr.disconnect (user, gspc::net::make::disconnect_frame ());
        }
        return 0;
      }

      template <class Proto>
      void base_server<Proto>::start_accept ()
      {
        m_new_connection.reset (new connection (m_io_service, *this));

        m_acceptor.async_accept ( m_new_connection->socket ()
                                , boost::bind ( &base_server<Proto>::handle_accept
                                              , this
                                              , boost::asio::placeholders::error
                                              )
                                );
      }

      template <class Proto>
      void
      base_server<Proto>::handle_accept (boost::system::error_code const &ec)
      {
        if (not ec)
        {
          m_new_connection->start ();
        }

        start_accept ();
      }
    }
  }
}
