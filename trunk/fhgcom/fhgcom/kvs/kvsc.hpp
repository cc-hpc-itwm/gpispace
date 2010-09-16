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
            kvs_.start (server_address, server_port);
          }
          void stop ()
          {
            kvs_.stop();
          }

          template <typename Val>
          void put (key_type const & k, Val v)
          {
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
            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::del( k )
                    , m
                    );
            DLOG(TRACE, "del(" << k << ") := " << m);
          }

          void save () const
          {
            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::msg_save()
                    , m
                    );
            DLOG(TRACE, "save() := " << m);
          }

          void load ()
          {
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
            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::clear()
                    , m
                    );
            DLOG(TRACE, "clear( "<< regexp << ") := " << m);
          }
        private:
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
    }
  }
}
#endif
