#include "queue_manager.hpp"

#include <cerrno>

#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include <gspc/net/frame.hpp>
#include <gspc/net/frame_builder.hpp>

#include <gspc/net/error.hpp>
#include <gspc/net/user.hpp>
#include <gspc/net/server/subscription.hpp>

#include <gspc/net/server/service_demux.hpp>
#include <gspc/net/server/default_service_demux.hpp>

#include <gspc/net/auth/default_auth.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      queue_manager_t::queue_manager_t (service_demux_t &demux)
        : m_mutex ()
        , m_subscriptions ()
        , m_user_subscriptions ()
        , m_service_demux (demux)
      {}

      queue_manager_t::~queue_manager_t ()
      {
        user_subscription_map_t::iterator user_it =
          m_user_subscriptions.begin ();
        const user_subscription_map_t::iterator user_it_end =
          m_user_subscriptions.end ();

        for (; user_it != user_it_end ; ++user_it)
        {
          BOOST_FOREACH (subscription_ptr sub, user_it->second)
          {
            // remove subscription
            subscription_map_t::iterator sub_map_it =
              m_subscriptions.find (sub->destination);
            if (sub_map_it != m_subscriptions.end ())
            {
              sub_map_it->second.remove (sub);
            }
            if (sub_map_it->second.empty ())
            {
              m_subscriptions.erase (sub_map_it);
            }
          }
        }
      }

      bool queue_manager_t::is_connected (user_ptr user) const
      {
        shared_lock lock (m_mutex);
        return m_connections.find (user) != m_connections.end ();
      }

      static void s_maybe_send_receipt (user_ptr user, frame const &f)
      {
        gspc::net::header::receipt r (f);
        if (r.value ())
        {
          //! \todo RV explain why the return value is ignored
          user->deliver (make::receipt_frame (r));
        }
      }

      int
      queue_manager_t::handle_frame (user_ptr user, frame const &f)
      {
        if      (f.get_command () == "CONNECT")
        {
          return connect (user, f);
        }

        if (not is_connected (user))
        {
          user->deliver
            (gspc::net::make::error_frame ( f
                                          , gspc::net::E_UNAUTHORIZED
                                          , "you are not connected"
                                          )
            );
          return -EPERM;
        }

        if (f.get_command () == "SEND")
        {
          if (not f.get_header ("destination"))
          {
            user->deliver
              (gspc::net::make::error_frame ( f
                                            , gspc::net::E_BAD_REQUEST
                                            , "required header 'destination' is missing"
                                            )
              );
            return -EPROTO;
          }
          return send (user, *f.get_header ("destination"), f);
        }
        else if (f.get_command () == "SUBSCRIBE")
        {
          if (not f.get_header ("destination"))
          {
            user->deliver
              (gspc::net::make::error_frame ( f
                                            , gspc::net::E_BAD_REQUEST
                                            , "required header 'destination' is missing"
                                            )
              );
            return -EPROTO;
          }
          if (not f.get_header ("id"))
          {
            user->deliver
              (gspc::net::make::error_frame ( f
                                            , gspc::net::E_BAD_REQUEST
                                            , "required header 'id' is missing"
                                            )
              );
            return -EPROTO;
          }

          return subscribe ( user
                           , *f.get_header ("destination")
                           , *f.get_header ("id")
                           , f
                           );
        }
        else if (f.get_command () == "UNSUBSCRIBE")
        {
          if (not f.get_header ("id"))
          {
            user->deliver
              (gspc::net::make::error_frame ( f
                                            , gspc::net::E_BAD_REQUEST
                                            , "required header 'id' is missing"
                                            )
              );
            return -EPROTO;
          }

          return unsubscribe ( user
                             , *f.get_header ("id")
                             , f
                             );
        }
        else if (f.get_command () == "DISCONNECT")
        {
          return disconnect (user, f);
        }
        else
        {
          user->deliver
            (gspc::net::make::error_frame ( f
                                          , gspc::net::E_BAD_REQUEST
                                          , "invalid command '" + f.get_command () + "'"
                                          )
            );
          return -EBADMSG;
        }
      }

      int
      queue_manager_t::handle_error ( user_ptr user
                                    , boost::system::error_code const &
                                    )
      {
        return disconnect (user);
      }

      int
      queue_manager_t::deliver (frame const &f)
      {
        if (not f.get_header ("destination"))
        {
          return -EPROTO;
        }

        shared_lock lock (m_mutex);

        subscription_map_t::iterator sub_it =
          m_subscriptions.find (*f.get_header ("destination"));

        if (sub_it != m_subscriptions.end ())
        {
          frame frame_to_deliver (f);
          frame_to_deliver.del_header ("reply-to");

          BOOST_FOREACH (subscription_ptr sub, sub_it->second)
          {
            frame_to_deliver.set_header ("subscription", sub->id);
            sub->user->deliver (frame_to_deliver);
          }
        }

        return 0;
      }

      int
      queue_manager_t::connect (user_ptr u, frame const &f)
      {
        unique_lock lock (m_mutex);

        if (m_connections.find (u) == m_connections.end ())
        {
          if ( auth::default_auth ().is_authorized
               (f.get_header_or ("cookie", std::string ("")))
             )
          {
            ++m_session_id;

            gspc::net::frame connected =
              make::connected_frame (gspc::net::header::version ("1.0"));

            connected.set_or_delete_header ("correlation-id", f.get_header ("message-id"));
            connected.set_header
              ( "session-id"
              , boost::format ("session-%1%-%2%") % time (NULL) % m_session_id
              );
            //! \todo RV do not ignore return value from deliver
            u->deliver (connected);

            m_connections.insert (u);

            return 0;
          }
          else
          {
            //! \todo RV do not ignore return value from deliver
            u->deliver (gspc::net::make::error_frame ( f
                                                     , gspc::net::E_UNAUTHORIZED
                                                     , "you shall not pass"
                                                     )
            );
            return -EPERM;
          }
        }
        else
        {
          return -EPROTO;
        }
      }

      int
      queue_manager_t::disconnect (user_ptr user)
      {
        unique_lock lock (m_mutex);

        m_connections.erase (user);

        user_subscription_map_t::iterator user_it =
          m_user_subscriptions.find (user);

        if (user_it == m_user_subscriptions.end ())
          return 0;

        BOOST_FOREACH (subscription_ptr sub, user_it->second)
        {
          // remove subscription
          subscription_map_t::iterator sub_map_it =
            m_subscriptions.find (sub->destination);
          if (sub_map_it != m_subscriptions.end ())
          {
            sub_map_it->second.remove (sub);
          }
          if (sub_map_it->second.empty ())
          {
            m_subscriptions.erase (sub_map_it);
          }
        }

        m_user_subscriptions.erase (user);

        //! \todo RV do not return const 0 and set return type to void
        return 0;
      }

      int
      queue_manager_t::disconnect (user_ptr user, frame const &f)
      {
        int rc = disconnect (user);

        s_maybe_send_receipt (user, f);

        return rc;
      }

      int queue_manager_t::send ( user_ptr user
                                , std::string const & dst
                                , frame const &f
                                )
      {
        if (dst.substr (0, 9) == "/service/")
        {
          return m_service_demux.handle_request (dst, f, this);
        }

        shared_lock lock (m_mutex);

        subscription_map_t::iterator sub_it =
          m_subscriptions.find (dst);

        if (sub_it != m_subscriptions.end ())
        {
          frame frame_to_deliver (f);
          frame_to_deliver.set_command ("MESSAGE");

          BOOST_FOREACH (subscription_ptr sub, sub_it->second)
          {
            frame_to_deliver.set_header ("subscription", sub->id);

            //! \todo RV explain why the return value is ignored
            sub->user->deliver (frame_to_deliver);
          }
        }

        s_maybe_send_receipt (user, f);

        //! \todo RV do not return const 0 and set return type to void
        return 0;
      }

      int queue_manager_t::subscribe ( user_ptr user
                                     , std::string const &dst
                                     , std::string const &id
                                     , frame const &f
                                     )
      {
        unique_lock lock (m_mutex);

        s_maybe_send_receipt (user, f);

        // check if we already have that subscription
        {
          user_subscription_map_t::iterator user_it =
            m_user_subscriptions.find (user);
          if (user_it != m_user_subscriptions.end ())
          {
            BOOST_FOREACH (subscription_ptr sub, user_it->second)
            {
              if (sub->id == id)
                return 0;
            }
          }
        }

        subscription_ptr sub (new subscription_t);
        if (not sub)
        {
          return -ENOMEM;
        }
        else
        {
          sub->user = user;
          sub->destination = dst;
          sub->id = id;

          m_subscriptions      [dst].push_back (sub);
          m_user_subscriptions [user].push_back (sub);
        }

        // sanity check the frame
        return 0;
      }

      //! \todo RV do not return const 0 and set return type to void
      int queue_manager_t::unsubscribe ( user_ptr user
                                       , std::string const & id
                                       , frame const &f
                                       )
      {
        unique_lock lock (m_mutex);

        user_subscription_map_t::iterator user_it =
          m_user_subscriptions.find (user);

        if (user_it == m_user_subscriptions.end ())
          return 0;

        subscription_ptr sub_to_remove;

        BOOST_FOREACH (subscription_ptr sub, user_it->second)
        {
          if (sub->id == id)
          {
            sub_to_remove = sub;
            user_it->second.remove (sub);
            break;
          }
        }

        if (not sub_to_remove)
          return 0;

        if (user_it->second.empty ())
        {
          m_user_subscriptions.erase (user_it);
        }

        // remove subscription
        subscription_map_t::iterator sub_map_it =
          m_subscriptions.find (sub_to_remove->destination);

        if (sub_map_it == m_subscriptions.end ())
        {
          return 0;
        }

        sub_map_it->second.remove (sub_to_remove);
        if (sub_map_it->second.empty ())
          m_subscriptions.erase (sub_map_it);

        s_maybe_send_receipt (user, f);

        return 0;
      }

      int queue_manager_t::ack (user_ptr u, frame const &)
      {
        return -ENOTSUP;
      }
    }
  }
}
