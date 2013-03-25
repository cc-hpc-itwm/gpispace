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
    server_t *s_new_unix_server ( std::string const & path
                                , server::queue_manager_t &qmgr
                                , boost::system::error_code & ec
                                )
    {
      using namespace boost::system;

      if (path.empty ())
      {
        ec = errc::make_error_code (errc::invalid_argument);
        return (server_t*)(0);
      }

      ec = errc::make_error_code (errc::not_supported);
      return (server_t*)(0);
    }

    static
    server_t *s_new_tcp_server ( std::string const & location
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
          return new gspc::net::server::tcp_server (host, port, qmgr);
        }
        catch (boost::system::system_error const &se)
        {
          ec = se.code ();
        }
      }

      return (server_t*)(0);
    }

    server_t *serve ( const char *url
                    , server::queue_manager_t &qmgr
                    )
    {
      boost::system::error_code ec;
      server_t *server = serve (url, qmgr, ec);

      if (ec)
      {
        if (server) delete server;
        throw boost::system::system_error (ec);
      }

      return server;
    }

    server_t *serve ( const char *url
                    , server::queue_manager_t & qmgr
                    , boost::system::error_code & ec
                    )
    {
      using namespace boost::system;

      std::string s_url (url);

      ec = errc::make_error_code (errc::success);

      if (s_url.find ("unix://") == 0)
      {
        return s_new_unix_server (s_url.substr (7), qmgr, ec);
      }
      else if (s_url.find ("tcp://") == 0)
      {
        return s_new_tcp_server (s_url.substr (6), qmgr, ec);
      }

      ec = errc::make_error_code (errc::wrong_protocol_type);

      return (server_t*)(0);
    }
  }
}
