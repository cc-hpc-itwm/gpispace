#ifndef FHG_COM_KVSD_HPP
#define FHG_COM_KVSD_HPP 1

#include <signal.h> /* For SIGTERM. */

#include <string>

#include <fhglog/fhglog.hpp>

#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_map.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>

#include <fhgcom/kvs/exception.hpp>
#include <fhgcom/kvs/message/type.hpp>

#include <fhgcom/session.hpp>
#include <fhgcom/session_manager.hpp>

#include <sstream>
#include <fstream>

#include <boost/serialization/map.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

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
            DLOG(TRACE, "put (" << m.entries().begin()->first << ") := " << m.entries().begin()->second);

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
            DLOG(TRACE, "get (" << m.key() << ")");
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
            DLOG(TRACE, "del (" << m.key() << ")");
            store_.del (m.key());
            return fhg::com::kvs::message::error ();
          }

          fhg::com::kvs::message::type
          operator () (fhg::com::kvs::message::msg_inc const & m)
          {
            DLOG(TRACE, "inc (" << m.key() << ", " << m.step() << ")");
            try
            {
              fhg::com::kvs::message::list list;
              list.entries()[ m.key() ] = store_.inc(m.key(), m.step());
              return list;
            }
            catch (std::exception const & ex)
            {
              return fhg::com::kvs::message::error
                (fhg::com::kvs::message::error::KVS_EUNKNOWN, ex.what());
            }
          }

          fhg::com::kvs::message::type
          operator () (fhg::com::kvs::message::msg_save const & m)
          {
            DLOG(TRACE, "save (" << m.file() << ")");
            try
            {
              if (m.file().empty())
                store_.save();
              else
                store_.save(m.file());
              return fhg::com::kvs::message::error ();
            }
            catch (std::exception const & ex)
            {
              return fhg::com::kvs::message::error
                (fhg::com::kvs::message::error::KVS_EUNKNOWN, ex.what());
            }
          }

          fhg::com::kvs::message::type
          operator () (fhg::com::kvs::message::clear const & m)
          {
            DLOG(TRACE, "clear (" << m.regexp() << ")");
            try
            {
              store_.clear (m.regexp());
              return fhg::com::kvs::message::error ();
            }
            catch (std::exception const & ex)
            {
              return fhg::com::kvs::message::error
                (fhg::com::kvs::message::error::KVS_EUNKNOWN, ex.what());
            }
          }

          fhg::com::kvs::message::type
          operator () (fhg::com::kvs::message::msg_load const & m)
          {
            DLOG(TRACE, "load (" << m.file() << ")");
            try
            {
              if (m.file().empty())
                store_.load();
              else
                store_.load(m.file());
              return fhg::com::kvs::message::error ();
            }
            catch (std::exception const & ex)
            {
              return fhg::com::kvs::message::error
                (fhg::com::kvs::message::error::KVS_EUNKNOWN, ex.what());
            }
          }

          fhg::com::kvs::message::type
          operator () (fhg::com::kvs::message::req_list const & m)
          {
            DLOG(TRACE, "list (" << m.regexp() << ")");
            fhg::com::kvs::message::list list;
            store_.entries (list.entries());
            return list;
          }

          fhg::com::kvs::message::type
          operator () (fhg::com::kvs::message::msg_term const & m)
          {
            DLOG(TRACE, "term (" << m.code() << ", " << m.reason() << ")");
            if (15 == m.code())
            {
              if (m.uid () != getuid ())
              {
                MLOG ( WARN
                     , "termination requested by client:"
                     << " code := " << m.code()
                     << " reason := " << m.reason ()
                     << " uid := " << m.uid ()
                     );
              }

              kill (getpid(), SIGTERM);
            }
            return fhg::com::kvs::message::error ();
          }

          fhg::com::kvs::message::type
          operator () (fhg::com::kvs::message::msg_ping const & m)
          {
            DLOG(TRACE, "ping ()");
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
        class kvsd : public session_manager<session>
        {
        private:
          template <typename T>
          struct entry_t
          {
            entry_t ()
            {}

            entry_t ( T const &value
                    , boost::posix_time::ptime exp = boost::posix_time::min_date_time
                    )
              : m_value (value)
              , m_expiry (exp)
            {}

            entry_t (entry_t const &other)
              : m_value (other.m_value)
              , m_expiry (other.m_expiry)
            {}

            entry_t & operator= (entry_t const &rhs)
            {
              m_value = rhs.m_value;
              m_expiry = rhs.m_expiry;
              return *this;
            }

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
          typedef boost::unordered_map<key_type, entry_type> store_type;
          typedef boost::unique_lock<boost::recursive_mutex> lock_t;

          explicit
          kvsd (const std::string & file = "", const bool write_through=false)
            : file_(file)
            , write_through_enabled_(write_through)
          {
            if (! file_.empty() && boost::filesystem::exists (file_))
            {
              try
              {
                LOG(DEBUG, "loading contents from file storage: " << file);
                load ();
              }
              catch (std::exception const & ex)
              {
                LOG (WARN, "could not load file storage: " << ex.what());
              }
            }
          }

          virtual ~kvsd()
          {
            write_through ();
          }

          bool toggle_write_through ()
          {
            bool old (write_through_enabled_);
            write_through_enabled_ = ! write_through_enabled_;
            return old;
          }

          void enable_write_through ()
          {
            write_through_enabled_ = true;
          }
          void disable_write_through ()
          {
            write_through_enabled_ = false;
          }

          bool is_write_through_enabled () const
          {
            return write_through_enabled_;
          }

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

            write_through ();
          }

          template <typename Val>
          void put ( key_type const & k, Val v
                   , boost::posix_time::ptime expiry = boost::posix_time::min_date_time
                   )
          {
            lock_t lock(mutex_);
            store_[ k ] = entry_type ( boost::lexical_cast<value_type>(v)
                                     , expiry
                                     );

            write_through ();
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

          value_type inc ( key_type const & k
                         , int step
                         )
          {
            lock_t lock(mutex_);

            int value = 0;

            store_type::iterator it (store_.find(k));
            if (it != store_.end())
            {
              value = boost::lexical_cast<int> (it->second.value ());
            }
            else
            {
              it = store_.insert ( it
                                 , store_type::value_type ( k
                                                          , entry_type ("0")
                                                          )
                                 );
            }

            value += step;

            it->second = entry_type (boost::lexical_cast<std::string> (value));

            return it->second.value ();
          }

          void clear (std::string const & /*regexp*/)
          {
            lock_t lock(mutex_);

            const std::size_t count (store_.size());

            store_.clear ();

            write_through ();

            LOG_IF(INFO, count > 0, "cleared " << count << " entries");
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

            write_through ();
          }

          void save () const
          {
            save (file_);
          }

          void save (std::string const & file) const
          {
            if (file.empty ()) return;

            std::ofstream ofs (file.c_str());
            if (ofs)
            {
              boost::archive::xml_oarchive ar(ofs);

              lock_t lock(mutex_);
              std::map<key_type, entry_type> tmp_map_ (store_.begin(), store_.end());
              ar << boost::serialization::make_nvp("kvsd", tmp_map_);

              MLOG (DEBUG, "saved " << store_.size() << " entries");
            }
            else
            {
              throw std::runtime_error ("could not save to file: " + file);
            }
          }

          void load ()
          {
            load (file_);
          }

          void load (std::string const & file)
          {
            std::ifstream ifs (file.c_str());
            if (ifs)
            {
              boost::archive::xml_iarchive ar(ifs);
              std::map<key_type, entry_type> tmp_map_;
              ar >> boost::serialization::make_nvp("kvsd", tmp_map_);

              lock_t lock(mutex_);
              store_.clear ();
              store_.insert (tmp_map_.begin(), tmp_map_.end());

              LOG(INFO, "loaded " << store_.size() << " entries");
            }
            else
            {
              throw std::runtime_error ("could not load from file: " + file);
            }
          }

          void entries (std::map<std::string, std::string> & m) const
          {
            lock_t lock(mutex_);
            std::list <std::string> to_delete;
            for ( store_type::const_iterator e (store_.begin())
                ; e != store_.end()
                ; ++e
                )
            {
              if (not e->second.is_expired ())
                m[e->first] = e->second.value ();
              else
                to_delete.push_back (e->first);
            }

            while (not to_delete.empty ())
            {
              const_cast<kvsd*>(this)->del (to_delete.front ());
              to_delete.pop_front ();
            }
          }
        protected:
          void on_add_hook (session_ptr) {}
          void on_del_hook (session_ptr) {}
          void on_data_hook (session_ptr client, const std::string &s)
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
              DMLOG ( WARN
                    , "closing connection: could not handle incoming data: "
                    << ex.what()
                    );
              client->close ();
            }
          }
        private:
          void write_through ()
          {
            if (write_through_enabled_)
            {
              try
              {
                save ();
              }
              catch (std::exception const & ex)
              {
                LOG(WARN, "write-through failed: " << ex.what());
              }
            }
          }

          mutable boost::recursive_mutex mutex_;
          std::string file_;
          store_type store_;
          bool write_through_enabled_;
        };
      }
    }
  }
}
#endif
