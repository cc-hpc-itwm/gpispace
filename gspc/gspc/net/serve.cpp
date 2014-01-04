#include <gspc/net/serve.hpp>

#include <string>

#include <boost/filesystem.hpp>
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

namespace gspc
{
  namespace net
  {
    namespace
    {
      server_ptr_t new_unix_server ( std::string const & location
                                   , server::queue_manager_t &qmgr
                                   )
      {
        boost::filesystem::remove (location);

        server::unix_server::endpoint_type ep
          (resolver<server::unix_server::protocol_type>::resolve (location));

        return server_ptr_t (new server::unix_server (gspc::net::io (), ep, qmgr));
      }

      server_ptr_t new_tcp_server ( std::string const & location
                                  , server::queue_manager_t &qmgr
                                  )
      {
        server::tcp_server::endpoint_type ep
          (resolver<server::tcp_server::protocol_type>::resolve (location));

        return server_ptr_t (new server::tcp_server (gspc::net::io (), ep, qmgr));
      }
    }

    server_ptr_t serve ( std::string const &url_s
                       , server::queue_manager_t & qmgr
                       )
    {
      gspc::net::initialize ();

      const fhg::util::url_t url (url_s);

      server_ptr_t server
        ( url.type() == "unix" ? new_unix_server (boost::filesystem::absolute (url.path()).string(), qmgr)
        : url.type() == "tcp" ? new_tcp_server (url.path(), qmgr)
        : throw boost::system::system_error
            (boost::system::errc::make_error_code (boost::system::errc::wrong_protocol_type))
        );

      server->set_queue_length
        (get_option ( url.args()
                    , "queue_length"
                    , limits::max_pending_frames_per_connection ()
                    )
        );

      server->start ();

      return server;
    }
  }
}
