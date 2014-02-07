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
                       )
    {
      client->set_timeout
        (get_option ( opts
                    , "timeout"
                    , -1
                    )
        );
      client->set_connect_timeout
        (get_option ( opts
                    , "connect_timeout"
                    , gspc::net::constants::CONNECT_TIMEOUT ()
                    )
        );
    }

    static
    client_ptr_t s_new_unix_client ( boost::asio::io_service & io
                                   , std::string const & location
                                   , option_map_t const &opts
                                   )
    {
      client::unix_client::endpoint_type ep
        (resolver<client::unix_client::protocol_type>::resolve (location));


        client::unix_client *c = new client::unix_client (io, ep);
        s_set_options (c, opts);
        return client_ptr_t (c);
    }

    static
    client_ptr_t s_new_tcp_client ( boost::asio::io_service & io
                                  , std::string const & location
                                  , option_map_t const &opts
                                  )
    {
      client::tcp_client::endpoint_type ep
        (resolver<client::tcp_client::protocol_type>::resolve (location));

      client::tcp_client *c = new client::tcp_client (io, ep);
        s_set_options (c, opts);
        return client_ptr_t (c);
    }

    client_ptr_t dial (std::string const &url_s)
    {
      using namespace boost::system;

      client_ptr_t client;

      const fhg::util::url_t url (url_s);

      if (url.type () == "unix")
      {
        client = s_new_unix_client ( gspc::net::io ()
                                   , boost::filesystem::absolute (url.path ()).string()
                                   , url.args ()
                                   );
      }
      else if (url.type () == "tcp")
      {
        client = s_new_tcp_client ( gspc::net::io ()
                                  , url.path ()
                                  , url.args ()
                                  );
      }
      else
      {
        throw boost::system::system_error (errc::make_error_code (errc::wrong_protocol_type));
      }

      if (client)
      {
        int rc;

        rc = client->start ();
        if (0 != rc)
        {
          if (-ECONNREFUSED == rc)
          {
            throw boost::system::system_error (errc::make_error_code (errc::connection_refused));
          }
          else if (-ETIME == rc)
          {
            throw boost::system::system_error (errc::make_error_code (errc::stream_timeout));
          }
          else if (-EPERM == rc)
          {
            throw boost::system::system_error (errc::make_error_code (errc::permission_denied));
          }
          else if (E_UNAUTHORIZED == rc)
          {
            throw boost::system::system_error (errc::make_error_code (errc::operation_not_permitted));
          }
          else
          {
            throw boost::system::system_error (errc::make_error_code (errc::protocol_error));
          }
        }
      }

      return client;
    }
  }
}
