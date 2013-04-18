#include "serve.hpp"

#include <string>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/system/system_error.hpp>

#include <fhg/util/url.hpp>

#include <gspc/net/limits.hpp>
#include <gspc/net/server.hpp>

#include <gspc/net/resolver.hpp>
#include <gspc/net/server/tcp_server.hpp>
#include <gspc/net/server/unix_server.hpp>

namespace gspc
{
  namespace net
  {
    typedef fhg::util::url_t::arg_map_t option_map_t;

    template <typename T>
    T s_get_option ( option_map_t const &opts
                   , std::string const & key
                   , T const &dflt
                   , boost::system::error_code & ec
                   )
    {
      using namespace boost::system;

      option_map_t::const_iterator it = opts.find (key);
      if (it != opts.end ())
      {
        try
        {
          return boost::lexical_cast<T>(it->second);
        }
        catch (boost::bad_lexical_cast const &)
        {
          ec = errc::make_error_code (errc::invalid_argument);
          return dflt;
        }
      }
      else
      {
        return dflt;
      }
    }

    template <typename Server>
    void s_set_options ( Server *server
                       , option_map_t const &opts
                       , boost::system::error_code & ec
                       )
    {
      server->set_queue_length
        (s_get_option ( opts
                      , "queue_length"
                      , limits::max_pending_frames_per_connection ()
                      , ec
                      )
        );
      server->set_thread_pool_size
        (s_get_option ( opts
                      , "threads"
                      , 1u
                      , ec
                      )
        );
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

        if (not ec)
        {
          server::unix_server *s = new server::unix_server (ep, qmgr);
          s_set_options (s, opts, ec);
          return server_ptr_t (s);
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

        if (not ec)
        {
          server::tcp_server *s = new server::tcp_server (ep, qmgr);
          s_set_options (s, opts, ec);
          return server_ptr_t (s);
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

    server_ptr_t serve ( std::string const &url_s
                       , server::queue_manager_t & qmgr
                       , boost::system::error_code & ec
                       )
    {
      using namespace boost::system;

      ec = errc::make_error_code (errc::success);

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
        ec = errc::make_error_code (errc::wrong_protocol_type);
      }

      if (server)
        server->start ();

      return server;
    }
  }
}
