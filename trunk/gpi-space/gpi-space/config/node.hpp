#ifndef GPI_SPACE_CONFIG_NODE_HPP
#define GPI_SPACE_CONFIG_NODE_HPP 1

#include <inttypes.h>
#include <string.h> // memset
#include <fhg/util/read_bool.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <gpi-space/config/config-data.hpp>
#include <boost/lexical_cast.hpp>
#include <sys/types.h>

namespace gpi_space
{
  namespace node
  {
    struct config
    {
      config ()
        : daemonize (false)
      {
        snprintf( kvs_url
                , gpi_space::MAX_HOST_LEN
                , "%s"
                , "localhost:2439"
                );
      }

      template <typename Mapping>
      void load (Mapping const & m)
      {
        daemonize = fhg::util::read_bool
          (m.get("node.daemonize", "false"));
        std::string default_url ("localhost:2439");
        {
          const char *kvs_url_env (getenv("KVS_URL"));
          if (kvs_url_env)
          {
             default_url = getenv("KVS_URL");
          }
        }
        snprintf ( kvs_url
                 , gpi_space::MAX_HOST_LEN
                 , "%s"
                 , m.get("kvs.url", default_url).c_str()
                 );
      }

      bool daemonize;
      char kvs_url[gpi_space::MAX_HOST_LEN];
    };

    void configure (config const & c)
    {
      try
      {
        setenv("KVS_URL", c.kvs_url, true);

        /* TODO: provide the following calls
              global should be struct
        fhg::com::kvs::global::create ();
        fhg::com::kvs::global::init ();
        fhg::com::kvs::global::get ();
        fhg::com::kvs::global::destroy ();
        */

        fhg::com::kvs::global::get_kvs_info().init ();

        // workaround until we have the above structure
        // put/del some entry to check the connection
        fhg::com::kvs::scoped_entry_t
            ( "kvs.connection.check"
            , boost::lexical_cast<std::string>(getpid())
            );
      }
      catch (std::exception const & ex)
      {
        LOG( ERROR
           , "Could not connect to KVS"
           << " at: " << c.kvs_url
           << " reason: " << ex.what()
           );
        throw std::runtime_error
            (std::string("kvs connection failed: ") + ex.what());
      }
    }
  }
}

#endif
