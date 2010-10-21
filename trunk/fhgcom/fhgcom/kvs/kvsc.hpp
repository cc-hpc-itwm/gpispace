#ifndef FHG_COM_KVSC_HPP
#define FHG_COM_KVSC_HPP 1

#include <string>

#include <fhglog/fhglog.hpp>

#include <fhgcom/tcp_client.hpp>

#include <fhgcom/kvs/store.hpp>
#include <fhgcom/kvs/message/type.hpp>
#include <fhgcom/kvs/message/show.hpp>

#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/thread.hpp>

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

          virtual ~kvsc()
          {}

          void start ( std::string const & server_address
                     , std::string const & server_port
                     )
          {
            boost::lock_guard<boost::mutex> lock (mtx_);

            kvs_.start (server_address, server_port);
          }
          void stop ()
          {
            boost::lock_guard<boost::mutex> lock (mtx_);

            kvs_.stop();
          }

          template <typename Val>
          void put (key_type const & k, Val v)
          {
            boost::lock_guard<boost::mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::put( k, v )
                    , m
                    );
            DLOG(TRACE, "put(" << k << ", " << v << ") := " << m);
          }

          fhg::com::kvs::message::list::map_type
          get(key_type const & k) const
          {
            boost::lock_guard<boost::mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::get(k)
                    , m
                    );
            DLOG(TRACE, "get(" << k << ") := " << m);
            return boost::get<fhg::com::kvs::message::list>(m).entries();
          }

          void del (key_type const & k)
          {
            boost::lock_guard<boost::mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::del( k )
                    , m
                    );
            DLOG(TRACE, "del(" << k << ") := " << m);
          }

          void save () const
          {
            boost::lock_guard<boost::mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::msg_save()
                    , m
                    );
            DLOG(TRACE, "save() := " << m);
          }

          void load ()
          {
            boost::lock_guard<boost::mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::msg_load()
                    , m
                    );
            DLOG(TRACE, "load() := " << m);
          }

          fhg::com::kvs::message::list::map_type
          list (std::string const & regexp = "")
          {
            boost::lock_guard<boost::mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::req_list(regexp)
                    , m
                    );
            DLOG(TRACE, "list(" << regexp << ") := " << m);
            return boost::get<fhg::com::kvs::message::list>(m).entries();
          }

          void clear (std::string const & regexp = "")
          {
            boost::lock_guard<boost::mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::clear()
                    , m
                    );
            DLOG(TRACE, "clear( "<< regexp << ") := " << m);
          }
        private:
          mutable boost::mutex mtx_;
          mutable tcp_client kvs_;

          template <typename Client>
          static
          void request ( Client & client
                       , fhg::com::kvs::message::type const & msg
                       , fhg::com::kvs::message::type & rpl
                       )
          {
            {
              std::stringstream sstr;
              {
                boost::archive::text_oarchive ar(sstr);
                ar & msg;
              }
              client.send (sstr.str(), boost::posix_time::seconds(10));
            }

            {
              std::stringstream sstr (client.recv(boost::posix_time::seconds(10)));
              {
                boost::archive::text_iarchive ar(sstr);
                ar & rpl;
              }
            }
          }
        };
      }

      client::kvsc * get_or_create_global_kvs (std::string const & host = "", std::string const & port = "")
      {
        static client::kvsc * global_kvsc_ (0);
        if (global_kvsc_ == NULL)
        {
          global_kvsc_ = new client::kvsc;

          std::string h ("localhost");
          if (host != "")
          {
            h = host;
          }
          else if (getenv("KVS_HOST") != NULL)
          {
            h = getenv("KVS_HOST");
          }

          std::string p ("2349");
          if (port != "")
          {
            p = port;
          }
          else if (getenv ("KVS_HOST") != NULL)
          {
            p = getenv("KVS_PORT");
          }

          global_kvsc_->start (host, port);
        }

        return global_kvsc_;
      }

      client::kvsc & global_kvs ()
      {
        return *get_or_create_global_kvs ();
      }

      template <typename Key, typename Val>
      inline
      void put (Key k, Val v)
      {
        return global_kvs().put(k,v);
      }

      template <typename Key>
      inline
      fhg::com::kvs::message::list::map_type get (Key k)
      {
        return global_kvs().get(k);
      }
    }
  }
}
#endif
