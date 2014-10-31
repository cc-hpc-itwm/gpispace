#ifndef FHG_COM_PEER_HPP
#define FHG_COM_PEER_HPP 1

#include <fhgcom/connection.hpp>
#include <fhgcom/header.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhgcom/peer_info.hpp>

#include <fhg/util/thread/event.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/system/error_code.hpp>
#include <boost/thread.hpp>

#include <deque>
#include <list>
#include <set>
#include <string>
#include <unordered_map>

namespace fhg
{
  namespace com
  {
    /*!
      This class abstracts from an endpoint
     */
    class peer_t : private boost::noncopyable
    {
    public:
      typedef std::function <void (boost::system::error_code const &)> handler_t;

      peer_t ( boost::asio::io_service&
             , std::string const & name
             , host_t const & host
             , port_t const & port
             , kvs::kvsc_ptr_t kvs_client
             , handler_t
             );

      virtual ~peer_t ();

      p2p::address_t const & address () const { return my_addr_; }
      boost::asio::ip::tcp::endpoint local_endpoint() const
      {
        return acceptor_.local_endpoint();
      }

      void start ();
      void stop ();
      void run ();

      p2p::address_t connect_to_via_kvs (std::string name);

      void async_send ( std::string const & to
                      , std::string const & data
                      , handler_t h
                      );
      void async_send ( p2p::address_t const& addr
                      , std::string const & data
                      , handler_t h
                      );
      void send ( std::string const & to
                , std::string const & data
                );
      void send ( p2p::address_t const& addr
                , std::string const & data
                );

      void async_recv
        ( message_t *m
        , std::function<void ( boost::system::error_code
                             , boost::optional<std::string> source_name
                             )
                       >
        );
      void recv (message_t *m);

    protected:
      void handle_hello_message (connection_t::ptr_t, const message_t *m);
      void handle_user_data   (connection_t::ptr_t, const message_t *m);
      void handle_error       (connection_t::ptr_t, const boost::system::error_code & error);

    private:
      struct to_send_t
      {
        to_send_t ()
          : message ()
          , handler()
        {}

        message_t  message;
        handler_t  handler;
      };

      struct to_recv_t
      {
        to_recv_t ()
          : message (nullptr)
          , handler()
        {}

        message_t  *message;
        std::function<void ( boost::system::error_code
                           , boost::optional<std::string> source_name
                           )
                     > handler;
      };

      struct connection_data_t
      {
        connection_data_t ()
          : send_in_progress (false)
          , name ("")
        {}

        bool send_in_progress;
        connection_t::ptr_t connection;
        std::string name;
        std::deque<to_send_t> o_queue;
      };

      void accept_new ();
      void handle_accept (const boost::system::error_code &);
      void update_my_location ();
      void renew_kvs_entries ();
      void connection_established (const p2p::address_t, boost::system::error_code const &);
      void handle_send (const p2p::address_t, const boost::system::error_code &);
      void start_sender (const p2p::address_t);

      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;

      mutable mutex_type mutex_;

      bool stopped_;
      bool stopping_;
      std::string name_;
      std::string host_;
      std::string port_;
      p2p::address_t my_addr_;
      fhg::util::thread::event<boost::system::error_code> started_;

      kvs::kvsc_ptr_t _kvs_client;

      boost::asio::io_service& io_service_;
      boost::asio::io_service::work io_service_work_;
      boost::asio::ip::tcp::acceptor acceptor_;
      boost::asio::deadline_timer m_renew_kvs_entries_timer;

      boost::shared_ptr<boost::thread> m_peer_thread;

      std::unordered_map<p2p::address_t, connection_data_t> connections_;

      std::set<connection_t::ptr_t> backlog_;

      connection_t::ptr_t listen_;

      std::list<to_recv_t> m_to_recv;
      std::list<std::pair<const message_t *, std::string /*remote_name*/>> m_pending;

      handler_t m_kvs_error_handler;
    };
  }
}

#endif
