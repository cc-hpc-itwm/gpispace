#include "serve.hpp"

#include <string>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/system/system_error.hpp>
#include <boost/system/error_code.hpp>

#include <fhg/util/url.hpp>

#include <gspc/net/io.hpp>
#include <gspc/net/option.hpp>
#include <gspc/net/limits.hpp>
#include <gspc/net/server.hpp>

#include <gspc/net/resolver.hpp>
#include <gspc/net/server/tcp_server.hpp>
#include <gspc/net/server/unix_server.hpp>
#include <gspc/net/server/default_queue_manager.hpp>

namespace gspc
{
  namespace net
  {
    template <typename Server>
    void s_set_options ( Server *server
                       , option_map_t const &opts
                       )
    {
      boost::system::error_code ec;
      server->set_queue_length
        (get_option ( opts
                    , "queue_length"
                    , limits::max_pending_frames_per_connection ()
                    , ec
                    )
        );
      if (ec)
      {
        throw boost::system::system_error (ec);
      }
    }

    static
    server_ptr_t s_new_unix_server ( std::string const & path
                                   , option_map_t const &opts
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

        if (ec)
        {
          throw boost::system::system_error (ec);
        }
        else
        {
          server_ptr_t s (new server::unix_server (gspc::net::io (), ep, qmgr));
          s_set_options (s.get(), opts);
          return s;
        }
      }
      catch (boost::system::system_error const &se)
      {
        throw;
      }

      return server_ptr_t ();
    }

    static
    server_ptr_t s_new_tcp_server ( std::string const & location
                                  , option_map_t const &opts
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

        if (ec)
        {
          throw boost::system::system_error (ec);
        }
        else
        {
          server_ptr_t s (new server::tcp_server (gspc::net::io (), ep, qmgr));
          s_set_options (s.get(), opts);
          return s;
        }
      }
      catch (boost::system::system_error const &se)
      {
        throw;
      }

      return server_ptr_t ();
    }

    server_ptr_t serve ( std::string const &url_s
                       , server::queue_manager_t & qmgr
                       )
    {
      gspc::net::initialize ();

      boost::system::error_code ec;

      server_ptr_t server;

      const fhg::util::url_t url (url_s);

      if (url.type () == "unix")
      {
        server = s_new_unix_server (url.path (), url.args (), qmgr, ec);
      }
      else if (url.type () == "tcp")
      {
        server = s_new_tcp_server (url.path (), url.args (), qmgr, ec);
      }
      else
      {
        throw boost::system::system_error
          (boost::system::errc::make_error_code (boost::system::errc::wrong_protocol_type));
      }

      if (ec)
      {
        throw boost::system::system_error (ec);
      }

      if (server)
        server->start ();

      return server;
    }
  }
}
