#ifndef FHG_COM_KVSC_HPP
#define FHG_COM_KVSC_HPP 1

#include <string>

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
          {
            DLOG(TRACE, "kvsc destructor");
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);
          }

          void start ( std::string const & server_address
                     , std::string const & server_port
                     , const bool auto_reconnect = true
                     , const boost::posix_time::time_duration timeout = boost::posix_time::seconds (120)
                     , const std::size_t max_connection_attempts = 11
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
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::put( e )
                    , m
                    );
            DLOG(TRACE, "put(...) := " << m);
          }
          template <typename Val>
          void put (key_type const & k, Val v)
          {
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

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
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

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
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::del( k )
                    , m
                    );
            DLOG(TRACE, "del(" << k << ") := " << m);
          }

          void save () const
          {
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::msg_save()
                    , m
                    );
            DLOG(TRACE, "save() := " << m);
          }

          void load ()
          {
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

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
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

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
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::clear()
                    , m
                    );
            DLOG(TRACE, "clear( "<< regexp << ") := " << m);
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
                                                     , boost::posix_time::seconds(10)
                                                     )
                                    );
            {
              boost::archive::text_iarchive ar(i_sstr);
              ar & rpl;
            }
          }
        };
      }

      // helper struct to initialize the global kvs in a thread safe way
      struct kvs_mgr
      {
        explicit
        kvs_mgr ( std::string const & host
                , std::string const & port
                , const bool auto_reconnect
                , const boost::posix_time::time_duration timeout
                , const std::size_t max_connection_attempts
                )
          : kvsc_(new client::kvsc)
        {
          std::string h (host);
          std::string p (port);

          if (h.empty() && p.empty())
          {
            if (getenv("KVS_URL"))
            {
              peer_info_t pi = peer_info_t::from_string (getenv("KVS_URL"));
              h = pi.host();
              p = pi.port();
            }
          }

          if (h.empty())
          {
            h = "localhost";
          }

          if (p.empty())
          {
            // TODO configuration variables!
            p = "2439";
          }

          LOG(TRACE, "global kvs auto-configured to be at: [" << h << "]:" << p);
          kvsc_->start (h,p,auto_reconnect,timeout,max_connection_attempts);
        }

        ~kvs_mgr ()
        {
          // TODO: unfortunately, we cannot delete the kvsc here, since it might
          // still  be required  during program  shutdown  (statically allocated
          // memory  is deallocated  in  an unspecified  order,  I guess).  This
          // means, deleting the pointer here, might cause a segfault ;-(
          //
          // delete kvsc_; kvsc_ = 0;
        }

        client::kvsc & kvs()
        {
          return *kvsc_;
        }
      private:
        client::kvsc * kvsc_;
      };

      inline client::kvsc & get_or_create_global_kvs ( std::string const & host = ""
                                                     , std::string const & port = ""
                                                     , const bool auto_reconnect = true
                                                     , const boost::posix_time::time_duration timeout = boost::posix_time::seconds (120)
                                                     , const std::size_t max_connection_attempts = 11
                                                     )
      {
        static kvs_mgr m (host, port, auto_reconnect, timeout, max_connection_attempts);
        return m.kvs();
      }

      inline client::kvsc & global_kvs ()
      {
        return get_or_create_global_kvs ();
      }

      typedef fhg::com::kvs::message::list::map_type values_type;

      inline
      void put (values_type const & e)
      {
        return global_kvs().put(e);
      }

      template <typename Key, typename Val>
      inline
      void put (Key k, Val v)
      {
        return global_kvs().put(k,v);
      }

      template <typename Key>
      inline
      void del (Key k)
      {
        return global_kvs().del (k);
      }

      template <typename Key>
      inline
      values_type get_tree (Key k)
      {
        return global_kvs().get(k);
      }

      template <typename Val, typename Key>
      inline
      Val get (Key k)
      {
        values_type v (get_tree(k));
        if (v.size() == 1)
        {
          return boost::lexical_cast<Val>(v.begin()->second);
        }
        else
        {
          throw std::runtime_error("kvs::get: returned 0 or more than 1 element");
        }
      }
    }
  }
}
#endif
