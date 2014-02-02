#ifndef FHG_COM_KVSC_HPP
#define FHG_COM_KVSC_HPP 1

#include <string>

#include <fhg/assert.hpp>
#include <fhglog/fhglog.hpp>

#include <fhgcom/peer_info.hpp>
#include <fhgcom/tcp_client.hpp>

#include <fhgcom/kvs/store.hpp>
#include <fhgcom/kvs/message/type.hpp>
#include <fhgcom/kvs/message/show.hpp>

#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>

namespace fhg
{
  namespace com
  {
    namespace kvs
    {
      namespace client
      {
        class kvsc
        {
        public:
          typedef std::string key_type;

          //! \todo remove empty-ctor+start() pattern
          kvsc(){}

          kvsc ( std::string const & server_address
               , std::string const & server_port
               , const bool auto_reconnect = true
               , const boost::posix_time::time_duration timeout = boost::posix_time::seconds (120)
               , const std::size_t max_connection_attempts = 3
               )
          {
            kvs_.start ( server_address, server_port
                       , auto_reconnect
                       , timeout
                       , max_connection_attempts
                       );
          }

          virtual ~kvsc()
          {
            stop();
          }

          void start ( std::string const & server_address
                     , std::string const & server_port
                     , const bool auto_reconnect = true
                     , const boost::posix_time::time_duration timeout = boost::posix_time::seconds (120)
                     , const std::size_t max_connection_attempts = 3
                     )
          {
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

            kvs_.start ( server_address, server_port
                       , auto_reconnect
                       , timeout
                       , max_connection_attempts
                       );
          }

          void stop ()
          {
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

            kvs_.stop();
          }

          void put (fhg::com::kvs::message::put::map_type const & e)
          {
            timed_put (e, 0);
          }

          void timed_put ( fhg::com::kvs::message::put::map_type const & e
                         , size_t expiry
                         )
          {
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::put (e).set_expiry (expiry)
                    , m
                    );
          }

          template <typename Val>
          void put (key_type const & k, Val v)
          {
            this->timed_put<Val>(k, v, 0);
          }

          template <typename Val>
          void timed_put (key_type const & k, Val v, size_t expiry)
          {
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::put (k, v).set_expiry (expiry)
                    , m
                    );
          }

          fhg::com::kvs::message::list::map_type
          get(key_type const & k) const
          {
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::msg_get(k)
                    , m
                    );
            return boost::get<fhg::com::kvs::message::list>(m).entries();
          }

          int
          inc(key_type const & k, int step) const
          {
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::msg_inc(k, step)
                    , m
                    );
            return boost::lexical_cast<int>
              (boost::get<fhg::com::kvs::message::list>(m).entries().begin()->second);
          }

          void del (key_type const & k)
          {
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::del( k )
                    , m
                    );
          }

          void save () const
          {
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::msg_save()
                    , m
                    );
          }

          void load ()
          {
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::msg_load()
                    , m
                    );
          }

          fhg::com::kvs::message::list::map_type
          list (std::string const & regexp = "")
          {
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::req_list(regexp)
                    , m
                    );
            return boost::get<fhg::com::kvs::message::list>(m).entries();
          }

          void clear()
          {
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::clear()
                    , m
                    );
          }

          void term (int code, std::string const & reason)
          {
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::msg_term(code, reason)
                    , m
                    );
          }

          bool ping ()
          {
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            try
            {
              request ( kvs_
                      , fhg::com::kvs::message::msg_ping()
                      , m
                      );
              return true;
            }
            catch (std::exception const &)
            {
              return false;
            }
          }
        private:
          mutable boost::recursive_mutex mtx_;
          mutable tcp_client kvs_;

          template <typename Client>
          static
          void request ( Client & client
                       , fhg::com::kvs::message::type const & msg
                       , fhg::com::kvs::message::type & rpl
                       )
          {
            std::stringstream o_sstr;
            {
              boost::archive::text_oarchive ar(o_sstr);
              ar & msg;
            }
            std::stringstream i_sstr( client.request ( o_sstr.str()
                                                     , boost::posix_time::pos_infin
                                                     )
                                    );
            {
              boost::archive::text_iarchive ar(i_sstr);
              ar & rpl;
            }
          }
        };
      }

      typedef boost::shared_ptr<client::kvsc> kvsc_ptr_t;

      struct kvs_data
      {
        kvs_data ()
          : is_configured (false)
          , timeout (boost::posix_time::seconds (120))
          , max_connection_attempts (3)
          , m_auto_reconnect (true)
        {}

        void init ( std::string const & p_host = ""
                  , std::string const & p_port = ""
                  , const boost::posix_time::time_duration p_timeout = boost::posix_time::seconds (120)
                  , const std::size_t p_max_connection_attempts = 3
                  , bool auto_reconnect = true
                  )
        {
          bool modified (false);

          m_auto_reconnect = auto_reconnect;

          if (p_host.empty() || p_port.empty())
          {
            if (host.empty () || port.empty())
            {
              if (getenv("KVS_URL"))
              {
                peer_info_t pi = peer_info_t::from_string (getenv("KVS_URL"));
                host = pi.host();
                port = pi.port();
              }
              else
              {
                host = "localhost";
                port = "2439";
              }

              modified = true;
            }
            else
            {
              // keep already stored ones
            }
          }
          else
          {
            host = p_host;
            port = p_port;
            timeout = p_timeout;
            max_connection_attempts = p_max_connection_attempts;

            modified = true;
          }

          assert (! host.empty());
          assert (! port.empty());

          is_configured = true;

          if (modified)
          {
            DLOG (TRACE, "global kvs configured to be at: [" << host << "]:" << port);
            client = kvsc_ptr_t (new client::kvsc);
            start ();
          }
        }

        void start ()
        {
          if (! is_configured)
            init();
          client->start ( host , port
                        , m_auto_reconnect // autoconnect
                        , timeout
                        , max_connection_attempts
                        );
          is_started = true;
        }

        void stop ()
        {
          client.reset ();
          is_configured = false;
          is_started = false;
        }

        bool is_configured;
        bool is_started;
        boost::posix_time::time_duration timeout;
        std::size_t max_connection_attempts;
        bool        m_auto_reconnect;

        std::string host;
        std::string port;
        kvsc_ptr_t  client;
        boost::recursive_mutex mutex;
      };

      struct global
      {
        static void start ()
        {
          get_kvs_info ().start ();
        }

        static void stop ()
        {
          get_kvs_info ().stop ();
        }

        static void restart ()
        {
          stop ();
          start ();
        }

        static kvs_data & get_kvs_info ()
        {
          return **get_kvs_info_ptr ();
        }

        static kvs_data **get_kvs_info_ptr ()
        {
          static kvs_data *d = 0;
          if (d == 0)
            d = new kvs_data;
          return &d;
        }
      };

      inline kvsc_ptr_t get_or_create_global_kvs ( std::string const & host = ""
                                                 , std::string const & port = ""
                                                 , const bool = true
                                                 , const boost::posix_time::time_duration timeout = boost::posix_time::seconds (120)
                                                 , const std::size_t max_connection_attempts = 3
                                                 )
      {
        fhg::com::kvs::global::get_kvs_info().init( host
                                                  , port
                                                  , timeout
                                                  , max_connection_attempts
                                                  );
        fhg::com::kvs::global::start();
        return global::get_kvs_info ().client;
      }

      inline kvsc_ptr_t global_kvs ()
      {
        return global::get_kvs_info ().client;
      }

      typedef fhg::com::kvs::message::list::map_type values_type;

      struct scoped_entry_t : boost::noncopyable
      {
      public:
        template <typename Val>
        scoped_entry_t ( kvsc_ptr_t kvs_client
                       , std::string const & k
                       , Val v
                       )
          : _kvs_client (kvs_client)
          , key(k)
        {
          _kvs_client->put (key, v);
        }

        ~scoped_entry_t ()
        {
          _kvs_client->del (key);
        }

      private:
        kvsc_ptr_t _kvs_client;
        const std::string key;
      };
    }
  }
}
#endif
