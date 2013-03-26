#include "serve.hpp"

#include <string>

#include <boost/filesystem.hpp>
#include <boost/system/system_error.hpp>

#include <gspc/net/server.hpp>

#include <gspc/net/resolver.hpp>
#include <gspc/net/server/tcp_server.hpp>
#include <gspc/net/server/unix_server.hpp>

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
      namespace fs = boost::filesystem;

      try
      {
        fs::path full_path = fs::absolute (path);
        fs::remove (full_path);

        server::unix_server::endpoint_type ep;
        ep = resolver<server::unix_server::protocol_type>::resolve
          (full_path.string (), ec);

        if (not ec)
        {
          return server_ptr_t
            (new server::unix_server (ep, qmgr));
        }
      }
      catch (boost::system::system_error const &se)
      {
        ec = se.code ();
      }

      return server_ptr_t ();
    }

    static
    server_ptr_t s_new_tcp_server ( std::string const & location
                                  , server::queue_manager_t &qmgr
                                  , boost::system::error_code & ec
                                  )
    {
      try
      {
        server::tcp_server::endpoint_type ep;
        ep = resolver<server::tcp_server::protocol_type>::resolve ( location
                                                                  , ec
                                                                  );

        if (not ec)
        {
          return server_ptr_t
            (new server::tcp_server (ep, qmgr));
        }
      }
      catch (boost::system::system_error const &se)
      {
        ec = se.code ();
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

      ec = errc::make_error_code (errc::success);

      server_ptr_t server;

      if (url.find ("unix://") == 0)
      {
        server = s_new_unix_server (url.substr (7), qmgr, ec);
      }
      else if (url.find ("tcp://") == 0)
      {
        server = s_new_tcp_server (url.substr (6), qmgr, ec);
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
