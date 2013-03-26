#include "serve.hpp"

#include <string>

#include <boost/system/system_error.hpp>

#include <gspc/net/util.hpp>
#include <gspc/net/server.hpp>

#include <gspc/net/server/tcp_server.hpp>

namespace gspc
{
  namespace net
  {
    static
    server_ptr_t s_new_unix_server ( std::string const & path
                                   , server::queue_manager_t &qmgr
                                   , boost::system::error_code & ec
                                   )
    {
      using namespace boost::system;

      if (path.empty ())
      {
        ec = errc::make_error_code (errc::invalid_argument);
        return server_ptr_t ();
      }

      ec = errc::make_error_code (errc::not_supported);
      return server_ptr_t ();
    }

    static
    server_ptr_t s_new_tcp_server ( std::string const & location
                                  , server::queue_manager_t &qmgr
                                  , boost::system::error_code & ec
                                  )
    {
      using namespace boost::system;

      std::string host;
      std::string port;

      if (split_host_port (location, host, port) != std::string::npos)
      {
        ec = errc::make_error_code (errc::invalid_argument);
      }
      else
      {
        try
        {
          return server_ptr_t
            (new gspc::net::server::tcp_server (host, port, qmgr));
        }
        catch (boost::system::system_error const &se)
        {
          ec = se.code ();
        }
      }

      return server_ptr_t ();
    }

    server_ptr_t serve ( std::string const &url
                       , server::queue_manager_t &qmgr
                       )
    {
      boost::system::error_code ec;
      server_ptr_t server = serve (url, qmgr, ec);

      if (ec)
      {
        throw boost::system::system_error (ec);
      }

      return server;
    }

    server_ptr_t serve ( std::string const &url
                       , server::queue_manager_t & qmgr
                       , boost::system::error_code & ec
                       )
    {
      using namespace boost::system;

      std::string s_url (url);

      ec = errc::make_error_code (errc::success);

      server_ptr_t server;

      if (s_url.find ("unix://") == 0)
      {
        server = s_new_unix_server (s_url.substr (7), qmgr, ec);
      }
      else if (s_url.find ("tcp://") == 0)
      {
        server = s_new_tcp_server (s_url.substr (6), qmgr, ec);
      }
      else
      {
        ec = errc::make_error_code (errc::wrong_protocol_type);
      }

      if (server)
        server->start ();

      return server;
    }
  }
}
