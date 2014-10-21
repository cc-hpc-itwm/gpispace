#ifndef FHG_COM_KVSC_HPP
#define FHG_COM_KVSC_HPP 1

#include <fhgcom/kvs/message/type.hpp>
#include <fhgcom/peer_info.hpp>
#include <fhgcom/tcp_client.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/thread.hpp>

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

          kvsc ( boost::asio::io_service& io_service
               , std::string const & server_address
               , std::string const & server_port
               , const bool auto_reconnect = true
               , const boost::posix_time::time_duration timeout = boost::posix_time::seconds (120)
               , const std::size_t max_connection_attempts = 3
               )
            : kvs_ (io_service)
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

          void put (key_type const & k, std::string const& value)
          {
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::put (k, value)
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

          void del (key_type const & k)
          {
            boost::lock_guard<boost::recursive_mutex> lock (mtx_);

            fhg::com::kvs::message::type m;
            request ( kvs_
                    , fhg::com::kvs::message::del( k )
                    , m
                    );
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
    }
  }
}
#endif
