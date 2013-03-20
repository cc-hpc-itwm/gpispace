#include "queue_manager.hpp"

#include <cerrno>

#include <boost/foreach.hpp>

#include <gspc/net/frame.hpp>
#include <gspc/net/frame_builder.hpp>

#include <gspc/net/error.hpp>
#include <gspc/net/server/user.hpp>
#include <gspc/net/server/subscription.hpp>

#include <gspc/net/server/service_demux.hpp>
#include <gspc/net/server/default_service_demux.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      queue_manager_t::queue_manager_t ()
        : m_subscription_mutex ()
        , m_subscriptions ()
        , m_user_subscriptions ()
        , m_service_demux (gspc::net::server::default_service_demux())
      {}

      queue_manager_t::queue_manager_t (service_demux_t &demux)
        : m_subscription_mutex ()
        , m_subscriptions ()
        , m_user_subscriptions ()
        , m_service_demux (demux)
      {}

      queue_manager_t::~queue_manager_t ()
      {}

      int
      queue_manager_t::connect (user_ptr u, frame const &)
      {
        return 0;
      }

      int
      queue_manager_t::disconnect (user_ptr user, frame const &f)
      {
        unique_lock lock (m_subscription_mutex);

        user_subscription_map_t::iterator user_it =
          m_user_subscriptions.find (user);

        if (user_it == m_user_subscriptions.end ())
          return 0;

        BOOST_FOREACH (subscription_t *sub, user_it->second)
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
          delete sub;
        }
        m_user_subscriptions.erase (user);

        gspc::net::header::receipt r (f);
        if (r.value ())
        {
          user->deliver (make::receipt_frame (r));
        }

        return 0;
      }

      int queue_manager_t::send ( user_ptr user
                                , std::string const & dst
                                , frame const &f
                                )
      {
        int rc = 0;
        shared_lock lock (m_subscription_mutex);

        subscription_map_t::iterator sub_it =
          m_subscriptions.find (dst);

        if (sub_it != m_subscriptions.end ())
        {
          frame frame_to_deliver (f);
          frame_to_deliver.set_command ("MESSAGE");

          BOOST_FOREACH (subscription_t * sub, sub_it->second)
          {
            sub->user->deliver (frame_to_deliver);
          }
        }
        else
        {
          rc = -ESRCH;
        }

        gspc::net::header::receipt r (f);
        if (r.value ())
        {
          user->deliver (make::receipt_frame (r));
        }

        return rc;
      }

      int queue_manager_t::request ( user_ptr user
                                   , std::string const & dst
                                   , frame const & rqst
                                   )
      {
        int rc = 0;
        frame rply;

        rc = m_service_demux.handle_request (dst, rqst, rply);
        if (rc == 0)
        {
          rply.set_command ("REPLY");
        }
        else if (rc < 0)
        {
          rply = make::error_frame ( rc
                                   , strerror (-rc)
                                   );
        }

        user->deliver (rply);

        return rc;
      }

      int queue_manager_t::subscribe ( user_ptr user
                                     , std::string const &dst
                                     , std::string const &id
                                     )
      {
        int rc = 0;

        unique_lock lock (m_subscription_mutex);

        // check if we already have that subscription
        {
          user_subscription_map_t::iterator user_it =
            m_user_subscriptions.find (user);
          if (user_it != m_user_subscriptions.end ())
          {
            BOOST_FOREACH (subscription_t *sub, user_it->second)
            {
              if (sub->id == id)
                return 0;
            }
          }
        }

        subscription_t *sub = new subscription_t;
        if (0 == sub)
        {
          rc = -ENOMEM;
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
        return rc;
      }

      int queue_manager_t::unsubscribe ( user_ptr user
                                       , std::string const & id
                                       )
      {
        unique_lock lock (m_subscription_mutex);

        user_subscription_map_t::iterator user_it =
          m_user_subscriptions.find (user);

        if (user_it == m_user_subscriptions.end ())
          return 0;

        subscription_t *sub_to_remove = 0;

        BOOST_FOREACH (subscription_t *sub, user_it->second)
        {
          if (sub->id == id)
          {
            sub_to_remove = sub;
            user_it->second.remove (sub);
            break;
          }
        }

        if (0 == sub_to_remove)
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
          delete sub_to_remove;
          return 0;
        }

        sub_map_it->second.remove (sub_to_remove);
        if (sub_map_it->second.empty ())
          m_subscriptions.erase (sub_map_it);

        delete sub_to_remove;

        return 0;
      }

      int queue_manager_t::ack (user_ptr u, frame const &)
      {
        return -ENOTSUP;
      }
    }
  }
}
