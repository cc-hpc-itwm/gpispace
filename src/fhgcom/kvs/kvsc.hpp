#ifndef FHG_COM_KVSC_HPP
#define FHG_COM_KVSC_HPP 1

#include <fhgcom/kvs/message/type.hpp>
#include <fhgcom/peer_info.hpp>
#include <fhgcom/tcp_client.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>

#include <sstream>
#include <string>

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
