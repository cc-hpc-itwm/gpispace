#include "dial.hpp"

#include <string>

#include <boost/filesystem.hpp>
#include <boost/system/system_error.hpp>

#include <gspc/net/resolver.hpp>
#include <gspc/net/client.hpp>

namespace gspc
{
  namespace net
  {
    static
    client_ptr_t s_new_unix_client ( std::string const & path
                                   , boost::system::error_code & ec
                                   )
    {
      namespace fs = boost::filesystem;

      try
      {
        fs::path full_path = fs::canonical (path, ec);
        if (not ec)
        {
          return client_ptr_t ();
        }
      }
      catch (boost::system::system_error const &se)
      {
        ec = se.code ();
      }

      return client_ptr_t ();
    }

    static
    client_ptr_t s_new_tcp_client ( std::string const & location
                                  , boost::system::error_code & ec
                                  )
    {
      try
      {
        //        server::tcp_server::endpoint_type ep;
        //        ec = server::resolve_address (location, ep);

        if (not ec)
        {
          return client_ptr_t ();
        }
      }
      catch (boost::system::system_error const &se)
      {
        ec = se.code ();
      }

      return client_ptr_t ();
    }

    client_ptr_t dial (std::string const &url)
    {
      boost::system::error_code ec;
      client_ptr_t client = dial (url, ec);

      if (ec)
      {
        throw boost::system::system_error (ec);
      }

      return client;
    }

    client_ptr_t dial ( std::string const &url
                      , boost::system::error_code & ec
                      )
    {
      using namespace boost::system;

      ec = errc::make_error_code (errc::success);

      client_ptr_t client;

      if (url.find ("unix://") == 0)
      {
        client = s_new_unix_client (url.substr (7), ec);
      }
      else if (url.find ("tcp://") == 0)
      {
        client = s_new_tcp_client (url.substr (6), ec);
      }
      else
      {
        ec = errc::make_error_code (errc::wrong_protocol_type);
      }

      if (client)
        client->start ();

      return client;
    }
  }
}
