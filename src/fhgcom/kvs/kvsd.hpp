#ifndef FHG_COM_KVSD_HPP
#define FHG_COM_KVSD_HPP 1

#include <fhgcom/kvs/message/type.hpp>
#include <fhgcom/session.hpp>
#include <fhgcom/session_manager.hpp>

#include <fhglog/LogMacros.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/serialization/map.hpp>
#include <boost/thread.hpp>

#include <fstream>
#include <signal.h> /* For SIGTERM. */
#include <sstream>
#include <string>
#include <unordered_map>

namespace fhg
{
  namespace com
  {
    namespace kvs
    {
      namespace visitor
      {
        template <typename Store>
        struct message_handler : public boost::static_visitor<fhg::com::kvs::message::type>
        {
          message_handler (Store & s)
            : store_ (s)
          {}

          fhg::com::kvs::message::type
          operator () (fhg::com::kvs::message::put const & m)
          {
            boost::posix_time::ptime expiry
              = ( m.expiry ()

                ? boost::posix_time::microsec_clock::universal_time ()
                + boost::posix_time::microsec (m.expiry () * 1000)

                : boost::posix_time::min_date_time
                );

            store_.put (m.entries(), expiry);
            return fhg::com::kvs::message::error (); // no_error
          }

          fhg::com::kvs::message::type
          operator () (fhg::com::kvs::message::msg_get const & m)
          {
            try
            {
              fhg::com::kvs::message::list list;
              store_.get(m.key(), list.entries());
              return list;
            }
            catch (std::exception const & ex)
            {
              return fhg::com::kvs::message::error
                (fhg::com::kvs::message::error::KVS_EUNKNOWN, ex.what());
            }
          }

          fhg::com::kvs::message::type
          operator () (fhg::com::kvs::message::del const & m)
          {
            store_.del (m.key());
            return fhg::com::kvs::message::error ();
          }

          template <typename T>
          fhg::com::kvs::message::type
          operator () (T const &)
          {
            LOG(ERROR, "kvs server cannot handle this message!");
            return fhg::com::kvs::message::error
              (fhg::com::kvs::message::error::KVS_EINVAL, "STRANGE! message type unknown");
          }
        private:
          Store & store_;
        };
      }

      namespace server
      {
        class kvsd : public session_manager
        {
        private:
          template <typename T>
          struct entry_t
          {
            entry_t () = default;

            entry_t ( T const &value
                    , boost::posix_time::ptime exp = boost::posix_time::min_date_time
                    )
              : m_value (value)
              , m_expiry (exp)
            {}

            bool is_expired () const
            {
              if (m_expiry > boost::posix_time::min_date_time)
              {
                boost::posix_time::ptime now =
                  boost::posix_time::microsec_clock::universal_time ();
                return m_expiry < now;
              }
              return false;
            }

            T const & value () const { return m_value; }
            void set_value (T const &v) { m_value = v; }

            boost::posix_time::ptime const & expiry () const { return m_expiry; }
            void set_expiry (boost::posix_time::ptime e) { m_expiry = e; }
          private:
            friend class boost::serialization::access;
            template<typename Archive>
            void serialize (Archive & ar, const unsigned int /* version */ )
            {
              ar & BOOST_SERIALIZATION_NVP ( m_value );
              ar & BOOST_SERIALIZATION_NVP ( m_expiry );
            }

            T m_value;
            boost::posix_time::ptime m_expiry;
          };

        public:
          typedef std::size_t size_type;
          typedef std::string key_type;
          typedef std::string value_type;
          typedef entry_t<value_type> entry_type;
          typedef std::unordered_map<key_type, entry_type> store_type;
          typedef boost::unique_lock<boost::recursive_mutex> lock_t;

          virtual ~kvsd() = default;

          void put ( fhg::com::kvs::message::put::map_type const & m
                   , boost::posix_time::ptime expiry = boost::posix_time::min_date_time
                   )
          {
            lock_t lock(mutex_);
            for ( fhg::com::kvs::message::put::map_type::const_iterator e (m.begin())
                ; e != m.end()
                ; ++e
                )
            {
              store_[ e->first ] = entry_type (e->second, expiry);
            }
          }

          void get( key_type const & k
                  , fhg::com::kvs::message::list::map_type & m
                  ) const
          {
            lock_t lock(mutex_);

            std::list <std::string> to_delete;
            for (store_type::const_iterator e (store_.begin()); e != store_.end(); ++e)
            {
              if (e->first.substr(0, k.size()) == k)
              {
                if (not e->second.is_expired ())
                  m[e->first] = e->second.value ();
                else
                  to_delete.push_back (e->first);
              }
            }

            while (not to_delete.empty ())
            {
              const_cast<kvsd*>(this)->del (to_delete.front ());
              to_delete.pop_front ();
            }
          }

          void del (key_type const & k)
          {
            lock_t lock(mutex_);

            // TODO: work here
            bool changed (false);
            do
            {
              changed = false;
              for (store_type::iterator it (store_.begin()); it != store_.end(); ++it)
              {
                if (it->first.substr(0, k.size()) == k)
                {
                  //                  store_.quick_erase (it);
                  store_.erase (it);
                  changed = true;
                  break;
                }
              }
            } while (changed);
          }

        protected:
          virtual void on_data_hook
            (boost::shared_ptr<session> client, const std::string &s) override
          {
            try
            {
              fhg::com::kvs::message::type msg;
              { // deserialize
                std::stringstream sstr (s);
                boost::archive::text_iarchive ar(sstr);
                ar & msg;
              }

              // handle
              visitor::message_handler<kvsd> h (*this);
              fhg::com::kvs::message::type rpl(boost::apply_visitor (h, msg));

              { // serialize
                std::stringstream sstr;
                boost::archive::text_oarchive ar(sstr);
                ar & rpl;
                client->async_send (sstr.str());
              }
            }
            catch (std::exception const & ex)
            {
              client->close ();
            }
          }
        private:
          mutable boost::recursive_mutex mutex_;
          store_type store_;
        };
      }
    }
  }
}
#endif
