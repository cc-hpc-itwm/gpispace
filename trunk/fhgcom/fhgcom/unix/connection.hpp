#ifndef FHG_COM_UNIX_CONNECTION_HPP
#define FHG_COM_UNIX_CONNECTION_HPP 1

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/asio.hpp>

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)

namespace fhg
{
  namespace com
  {
    namespace unix
    {
      using boost::asio::local::stream_protocol;

      template <typename Manager, typename Parser>
      class connection : public boost::enable_shared_from_this<connection>
      {
      public:
        typedef Manager manager_type;
        typedef Parser  parser_type;
        typedef typename parser_type::message_type message_type;

        connection ( boost::asio::io_service & io_service
                   , manager_type & manager
                   );

        stream_protocol::socket & socket ()
        {
          return m_socket;
        }

        /*
        template <typename H>
        void async_send ( boost::buffer & buf, H h );
        template <typename H>
        void async_recv ( boost::buffer & buf, H h );
        */
      private:
        boost::asio::io_service & m_io_service;
        manager_type & m_manager;
        parser_type m_parser;   // work on incoming message
        message_type m_message; // incoming message
      };
    }
  }
}

#include "connection.tcc"

#else
#  error "Sorry, local sockets are not defined on this platform."
#endif

#endif
