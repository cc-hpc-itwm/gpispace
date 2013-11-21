#include "dial.hpp"

#include <string>

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/system/system_error.hpp>

#include <fhg/util/url.hpp>

#include <gspc/net/io.hpp>
#include <gspc/net/constants.hpp>
#include <gspc/net/option.hpp>
#include <gspc/net/resolver.hpp>
#include <gspc/net/client.hpp>

#include <gspc/net/client/tcp_client.hpp>
#include <gspc/net/client/unix_client.hpp>

namespace gspc
{
  namespace net
  {
    template <typename Client>
    void s_set_options ( Client *client
                       , option_map_t const &opts
                       , boost::system::error_code & ec
                       )
    {
      client->set_timeout
        (get_option ( opts
                    , "timeout"
                    , -1
                    , ec
                    )
        );
      client->set_heartbeat_info
        (get_option ( opts
                    , "heartbeat"
                    , heartbeat_info_t ("0,0")
                    , ec
                    )
        );
      client->set_connect_timeout
        (get_option ( opts
                    , "connect_timeout"
                    , gspc::net::constants::CONNECT_TIMEOUT ()
                    , ec
                    )
        );
    }

    static
    client_ptr_t s_new_unix_client ( boost::asio::io_service & io
                                   , std::string const & path
                                   , option_map_t const &opts
                                   , boost::system::error_code & ec
                                   )
    {
      namespace fs = boost::filesystem;
      using namespace gspc::net::client;

      try
      {
        fs::path full_path = fs::absolute (path);

        unix_client::endpoint_type ep;
        ep = resolver<unix_client::protocol_type>::resolve
          (full_path.string (), ec);
        if (ec)
          return client_ptr_t ();

        unix_client *c = new unix_client (io, ep);
        s_set_options (c, opts, ec);
        return client_ptr_t (c);
      }
      catch (boost::system::system_error const &se)
      {
        ec = se.code ();
      }

      return client_ptr_t ();
    }

    static
    client_ptr_t s_new_tcp_client ( boost::asio::io_service & io
                                  , std::string const & location
                                  , option_map_t const &opts
                                  , boost::system::error_code & ec
                                  )
    {
      using namespace gspc::net::client;

      try
      {
        tcp_client::endpoint_type ep;
        ep = resolver<tcp_client::protocol_type>::resolve ( location
                                                          , ec
                                                          );
        if (ec)
          return client_ptr_t ();

        tcp_client *c = new tcp_client (io, ep);
        s_set_options (c, opts, ec);
        return client_ptr_t (c);
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

    client_ptr_t dial ( std::string const &url_s
                      , boost::system::error_code & ec
                      )
    {
      gspc::net::initialize ();

      using namespace boost::system;

      ec = errc::make_error_code (errc::success);

      client_ptr_t client;

      const fhg::util::url_t url (url_s);

      if (url.type () == "unix")
      {
        client = s_new_unix_client ( gspc::net::io ()
                                   , url.path ()
                                   , url.args ()
                                   , ec
                                   );
      }
      else if (url.type () == "tcp")
      {
        client = s_new_tcp_client ( gspc::net::io ()
                                  , url.path ()
                                  , url.args ()
                                  , ec
                                  );
      }
      else
      {
        ec = errc::make_error_code (errc::wrong_protocol_type);
      }

      if (client)
      {
        int rc;

        rc = client->start ();
        if (0 != rc)
        {
          if (-ECONNREFUSED == rc)
          {
            ec = errc::make_error_code (errc::connection_refused);
          }
          else if (-ETIME == rc)
          {
            ec = errc::make_error_code (errc::stream_timeout);
          }
          else if (-EPERM == rc)
          {
            ec = errc::make_error_code (errc::permission_denied);
          }
          else if (E_UNAUTHORIZED == rc)
          {
            ec = errc::make_error_code (errc::operation_not_permitted);
          }
          else
          {
            ec = errc::make_error_code (errc::protocol_error);
          }
        }
      }

      return client;
    }
  }
}
