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
    static
    server_ptr_t s_new_unix_server ( std::string const & path
                                   , option_map_t const &opts
                                   , server::queue_manager_t &qmgr
                                   )
    {
      namespace fs = boost::filesystem;

      fs::path full_path = fs::absolute (path);
      fs::remove (full_path);

      server::unix_server::endpoint_type ep
        (resolver<server::unix_server::protocol_type>::resolve (full_path.string ()));

      return server_ptr_t (new server::unix_server (gspc::net::io (), ep, qmgr));
    }

    static
    server_ptr_t s_new_tcp_server ( std::string const & location
                                  , option_map_t const &opts
                                  , server::queue_manager_t &qmgr
                                  )
    {
      server::tcp_server::endpoint_type ep
        (resolver<server::tcp_server::protocol_type>::resolve (location));

      return server_ptr_t (new server::tcp_server (gspc::net::io (), ep, qmgr));
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

      server->set_queue_length
        (get_option ( opts
                    , "queue_length"
                    , limits::max_pending_frames_per_connection ()
                    )
        );

      server->start ();

      return server;
    }
  }
}
