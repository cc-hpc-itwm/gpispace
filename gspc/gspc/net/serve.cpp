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
    static void s_set_options ( Server *server
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
                                   )
    {
      namespace fs = boost::filesystem;

      fs::path full_path = fs::absolute (path);
      fs::remove (full_path);

      boost::system::error_code ec;

      server::unix_server::endpoint_type ep
        (resolver<server::unix_server::protocol_type>::resolve (full_path.string (), ec));

      if (ec)
      {
        throw boost::system::system_error (ec);
      }

      server_ptr_t s (new server::unix_server (gspc::net::io (), ep, qmgr));
      s_set_options (s.get(), opts);
      return s;
    }

    static
    server_ptr_t s_new_tcp_server ( std::string const & location
                                  , option_map_t const &opts
                                  , server::queue_manager_t &qmgr
                                  )
    {
      boost::system::error_code ec;

      server::tcp_server::endpoint_type ep
        (resolver<server::tcp_server::protocol_type>::resolve (location, ec));

      if (ec)
      {
        throw boost::system::system_error (ec);
      }

      server_ptr_t s (new server::tcp_server (gspc::net::io (), ep, qmgr));
      s_set_options (s.get(), opts);
      return s;
    }

    server_ptr_t serve ( std::string const &url_s
                       , server::queue_manager_t & qmgr
                       )
    {
      gspc::net::initialize ();

      const fhg::util::url_t url (url_s);

      server_ptr_t server
        ( url.type() == "unix" ? s_new_unix_server (url.path(), url.args(), qmgr)
        : url.type() == "tcp" ? s_new_tcp_server (url.path(), url.args(), qmgr)
        : throw boost::system::system_error
            (boost::system::errc::make_error_code (boost::system::errc::wrong_protocol_type))
        );

      if (server)
        server->start ();

      return server;
    }
  }
}
