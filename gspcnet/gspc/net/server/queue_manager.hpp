#ifndef GSPC_NET_SERVER_QUEUE_MANAGER_HPP
#define GSPC_NET_SERVER_QUEUE_MANAGER_HPP

#include <map>
#include <list>
#include <string>

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <gspc/net/frame_fwd.hpp>
#include <gspc/net/user_fwd.hpp>
#include <gspc/net/server/subscription_fwd.hpp>
#include <gspc/net/server/service_demux_fwd.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      class queue_manager_t
      {
      public:
        queue_manager_t ();
        queue_manager_t (service_demux_t & demux);

        ~queue_manager_t ();

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
        int request ( user_ptr u
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
        typedef std::list<subscription_t*> subscription_list_t;

        typedef std::map<std::string, subscription_list_t> subscription_map_t;
        typedef std::map<user_ptr, subscription_list_t>    user_subscription_map_t;

        //
        // MEMBER VARIABLES
        //
        mutable boost::shared_mutex m_subscription_mutex;

        // subscriptions
        //     if queue not already there, create it
        //        add user to queue
        subscription_map_t      m_subscriptions;
        user_subscription_map_t m_user_subscriptions;

        // used to handle requests
        service_demux_t & m_service_demux;
      };
    }
  }
}

#endif
