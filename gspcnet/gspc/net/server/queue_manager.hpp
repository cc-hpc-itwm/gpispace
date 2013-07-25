#ifndef GSPC_NET_SERVER_QUEUE_MANAGER_HPP
#define GSPC_NET_SERVER_QUEUE_MANAGER_HPP

#include <map>
#include <set>
#include <list>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <fhg/util/thread/atomic.hpp>

#include <gspc/net/frame_fwd.hpp>
#include <gspc/net/user.hpp>
#include <gspc/net/frame_handler.hpp>
#include <gspc/net/server/subscription_fwd.hpp>
#include <gspc/net/server/service_demux_fwd.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      class queue_manager_t : public frame_handler_t
                            , public user_t
      {
      public:
        queue_manager_t ();
        queue_manager_t (service_demux_t & demux);

        ~queue_manager_t ();

        int handle_frame (user_ptr, frame const &);
        int handle_error (user_ptr, boost::system::error_code const&);
        int deliver (frame const &f);

        /**
           Handle a CONNECT message
         */
        int connect (user_ptr u, frame const &);

        /**
           Handle a DISCONNECT message
        */
        int disconnect (user_ptr u, frame const &);
        int disconnect (user_ptr u);

        int send ( user_ptr u
                 , std::string const & dst
                 , frame const &
                 );

        /**
           subscribe to some event queue
         */
        int subscribe ( user_ptr u
                      , std::string const & dst
                      , std::string const & id
                      , frame const &
                      );

        /**
           unsubscribe from some event queue
        */
        int unsubscribe ( user_ptr u
                        , std::string const &id
                        , frame const &
                        );

        /**
           acknowledge the reception of a message
        */
        int ack (user_ptr u, frame const &);
      private:
        // disallow copy
        queue_manager_t (queue_manager_t const &);
        queue_manager_t & operator=(queue_manager_t const &);

        typedef boost::shared_lock<boost::shared_mutex> shared_lock;
        typedef boost::unique_lock<boost::shared_mutex> unique_lock;

        typedef std::list<user_ptr>        user_list_t;
        typedef boost::shared_ptr<subscription_t> subscription_ptr;
        typedef std::list<subscription_ptr> subscription_list_t;

        typedef std::map<std::string, subscription_list_t> subscription_map_t;
        typedef std::map<user_ptr, subscription_list_t>    user_subscription_map_t;

        typedef std::set<user_ptr> user_set_t;

        bool is_connected (user_ptr) const;

        //
        // MEMBER VARIABLES
        //
        mutable boost::shared_mutex m_mutex;

        user_set_t                  m_connections;
        fhg::thread::atomic<size_t> m_session_id;

        // subscriptions
        //     if queue not already there, create it
        //        add user to queue
        subscription_map_t      m_subscriptions;
        user_subscription_map_t m_user_subscriptions;

        // used to handle services
        service_demux_t & m_service_demux;
      };
    }
  }
}

#endif
