#include <gspc/kvs/net_service.hpp>

#include <gspc/net/error.hpp>
#include <gspc/net/frame_builder.hpp>
#include <gspc/net/server/default_queue_manager.hpp>
#include <gspc/net/server/queue_manager.hpp>

#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>

#include <fhg/assert.hpp>
#include <fhg/util/num.hpp>
#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/regex.hpp>
#include <boost/signals2.hpp>
#include <boost/thread/locks.hpp>

#include <iomanip>
#include <iostream>
#include <string>
#include <algorithm>

typedef boost::shared_lock<boost::shared_mutex> shared_lock;
typedef boost::unique_lock<boost::shared_mutex> unique_lock;

namespace gspc
{
  namespace kvs
  {
    kvs_t::kvs_t () {}
    kvs_t::kvs_t (std::string const &) {}

    int kvs_t::do_get (key_type const &key, value_type &val) const
    {
      purge_expired_keys ();

      shared_lock lock (m_mutex);

      value_map_t::const_iterator it = m_values.find (key);
      if (it == m_values.end ())
        return -ENOKEY;

      if (it->second->is_expired ())
      {
        unique_lock expired_lock (m_expired_entries_mutex);
        m_expired_entries.push_back (it->first);
        return -EKEYEXPIRED;
      }
      else
      {
        val = it->second->get_value ();
      }

      return 0;
    }

    int kvs_t::do_put (std::list<std::pair<key_type, value_type> > const &values)
    {
      purge_expired_keys ();

      typedef std::pair<key_type, value_type> kv_pair_t;
      {
        unique_lock lock (m_mutex);

        BOOST_FOREACH (kv_pair_t const &kp, values)
        {
          value_map_t::iterator it = m_values.find (kp.first);
          if (it == m_values.end ())
          {
            m_values.insert
              (std::make_pair ( kp.first
                              , entry_ptr_t (new entry_t (kp.second))
                              )
              );
          }
          else
          {
            it->second->set_value (kp.second);
          }
        }
      }

      BOOST_FOREACH (kv_pair_t const &kp, values)
      {
        notify (kp.first, E_PUT | E_EXIST);
      }

      return 0;
    }

    int kvs_t::do_get_regex ( std::string const &r
                         , std::list<std::pair<key_type, value_type> > &values
                         ) const
    {
      purge_expired_keys ();

      shared_lock lock (m_mutex);

      value_map_t::const_iterator it = m_values.begin ();
      const value_map_t::const_iterator end = m_values.end ();

      const boost::regex regex (r);

      while (it != end)
      {
        if (boost::regex_match (it->first, regex))
        {
          if (it->second->is_expired ())
          {
            unique_lock expired_lock (m_expired_entries_mutex);
            m_expired_entries.push_back (it->first);
          }
          else
          {
            values.push_back
              (std::make_pair (it->first, it->second->get_value ()));
          }
        }

        ++it;
      }

      return 0;
    }

    int kvs_t::do_del (key_type const &key)
    {
      purge_expired_keys ();

      {
        unique_lock lock (m_mutex);

        value_map_t::iterator it = m_values.find (key);
        if (it == m_values.end ())
          return -ENOKEY;

        m_values.erase (it);
      }

      notify (key, E_DEL);

      return 0;
    }

    int kvs_t::do_del_regex (std::string const &r)
    {
      purge_expired_keys ();

      std::list<key_type> deleted;

      {
        unique_lock lock (m_mutex);

        value_map_t::iterator it = m_values.begin ();
        const value_map_t::iterator end = m_values.end ();

        const boost::regex regex (r);

        while (it != end)
        {
          if (boost::regex_match (it->first, regex))
          {
            deleted.push_back (it->first);
            m_values.erase (it);
            it = m_values.begin ();
          }
          else
          {
            ++it;
          }
        }
      }

      BOOST_FOREACH (key_type const &key, deleted)
      {
        notify (key, E_DEL);
      }

      return 0;
    }

    int kvs_t::entry_t::is_popable () const
    {
      boost::unique_lock<boost::recursive_mutex> lock (mutex);

      const std::list<value_type> *alist =
        boost::get<std::list<value_type> >(&value);

      if (alist)
      {
        if (alist->empty ())
        {
          return -EAGAIN;
        }
        else
        {
          return 0;
        }
      }
      else
      {
        return -EINVAL;
      }
    }

    bool kvs_t::entry_t::is_expired () const
    {
      return
        (expiry == EXPIRES_NEVER) ? false
        : (expiry > time (NULL)) ? false
        : true;
    }

    void kvs_t::entry_t::expires_in (int ttl)
    {
      if (ttl != EXPIRES_NEVER)
        expiry = time (NULL) + ttl;
      else
        expiry = EXPIRES_NEVER;
    }

    void kvs_t::entry_t::set_value (kvs_t::value_type const &val)
    {
      boost::unique_lock<boost::recursive_mutex> lock (mutex);
      value = val;
    }

    kvs_t::value_type & kvs_t::entry_t::get_value ()
    {
      return value;
    }

    kvs_t::value_type const & kvs_t::entry_t::get_value () const
    {
      return value;
    }

    int kvs_t::entry_t::try_pop (value_type &val)
    {
      boost::unique_lock<boost::recursive_mutex> lock (mutex);

      std::list<value_type> *alist =
        boost::get<std::list<value_type> >(&value);

      if (alist)
      {
        if (alist->empty ())
        {
          return -EAGAIN;
        }
        else
        {
          val = alist->front (); alist->pop_front ();
          return 0;
        }
      }
      else
      {
        return -EINVAL;
      }
    }

    int kvs_t::entry_t::push (value_type const &val)
    {
      boost::unique_lock<boost::recursive_mutex> lock (mutex);

      std::list<value_type> *alist =
        boost::get<std::list<value_type> >(&value);

      if (alist)
      {
        alist->push_back (val);
        return 0;
      }
      else
      {
        return -EINVAL;
      }
    }

    int kvs_t::do_set_ttl (key_type const &key, int ttl)
    {
      purge_expired_keys ();

      unique_lock lock (m_mutex);

      value_map_t::iterator it = m_values.find (key);
      if (it == m_values.end ())
        return -ENOKEY;

      it->second->expires_in (ttl);

      return 0;
    }

    int kvs_t::do_set_ttl_regex (std::string const &r, int ttl)
    {
      purge_expired_keys ();

      unique_lock lock (m_mutex);

      value_map_t::iterator it = m_values.begin ();
      const value_map_t::iterator end = m_values.end ();

      const boost::regex regex (r);

      while (it != end)
      {
        if (boost::regex_match (it->first, regex))
        {
          it->second->expires_in (ttl);
        }

        ++it;
      }

      return 0;
    }

    int kvs_t::do_push (key_type const &key, value_type const &val)
    {
      purge_expired_keys ();

      {
        unique_lock lock (m_mutex);

        value_map_t::iterator it = m_values.find (key);
        if (it == m_values.end ())
        {
          std::list<value_type> alist;

          it = m_values.insert
            (std::make_pair ( key
                            , entry_ptr_t (new entry_t (alist))
                            )
            ).first;
        }

        int rc = it->second->push (val);
        if (rc != 0)
          return rc;
      }

      notify (key, E_PUSH | E_EXIST | E_POPABLE);

      return 0;
    }

    int kvs_t::do_try_pop (key_type const &key, value_type &val)
    {
      purge_expired_keys ();

      {
        unique_lock lock (m_mutex);

        value_map_t::iterator it = m_values.find (key);
        if (it == m_values.end ())
        {
          return -EAGAIN;
        }

        int rc = it->second->try_pop (val);
        if (rc != 0)
          return rc;
      }

      notify (key, E_POP);

      return 0;
    }

    int kvs_t::do_counter_reset (key_type const &key, int val)
    {
      purge_expired_keys ();

      {
        unique_lock lock (m_mutex);

        value_map_t::iterator it = m_values.find (key);
        if (it == m_values.end ())
        {
          m_values.insert
            (it, std::make_pair ( key
                                , entry_ptr_t (new entry_t (val))
                                )
            );
        }
        else
        {
          it->second->set_value (val);
        }
      }

      notify (key, E_PUT | E_EXIST);

      return 0;
    }

    int kvs_t::do_counter_change (key_type const &key, int &val, int delta)
    {
      purge_expired_keys ();

      {
        unique_lock lock (m_mutex);

        value_map_t::iterator it = m_values.find (key);
        if (it == m_values.end ())
        {
          it = m_values.insert
            (std::make_pair ( key
                            , entry_ptr_t (new entry_t (0))
                            )
            ).first;
        }

        int *cur = boost::get<int>(&it->second->get_value ());
        if (cur)
        {
          *cur += delta;
          val = *cur;
        }
        else
        {
          return -EINVAL;
        }
      }

      notify (key, E_PUT | E_EXIST);

      return 0;
    }

    int kvs_t::waiting_t::wait ()
    {
      boost::unique_lock<boost::mutex> lock (m_mutex);

      while (m_notified == 0)
      {
        m_cond.wait (lock);
      }

      return m_notified;
    }

    int kvs_t::waiting_t::wait (int timeout)
    {
      if (timeout == -1)
        return this->wait ();

      boost::system_time const deadline =
        boost::get_system_time() + boost::posix_time::milliseconds (timeout);

      boost::unique_lock<boost::mutex> lock (m_mutex);

      while (m_notified == 0)
      {
        if (not m_cond.timed_wait (lock, deadline))
        {
          if (0 == m_notified)
            return -ETIME;
        }
      }

      return m_notified;
    }

    void kvs_t::waiting_t::notify (int events)
    {
      if ((events & m_mask))
      {
        boost::unique_lock<boost::mutex> lock (m_mutex);
        m_notified |= events;
        m_cond.notify_one ();
      }
    }

    int kvs_t::do_wait ( key_type const &key
                       , int mask
                       , int timeout_in_ms
                       ) const
    {
      purge_expired_keys ();

      shared_lock glock (m_mutex);

      if (mask & (E_POPABLE | E_EXIST | E_DEL))
      {
        value_map_t::const_iterator it = m_values.find (key);

        if (it != m_values.end ())
        {
          if (mask & E_POPABLE)
          {
            if (0 == it->second->is_popable ())
            {
              return E_POPABLE;
            }
          }
          if (mask & E_EXIST)
          {
            return E_EXIST;
          }
        }
        else
        {
          if (mask & E_DEL)
          {
            return E_DEL;
          }
        }
      }

      waiting_t wobject (key, mask);

      {
        unique_lock wlock (m_waiting_mutex);
        m_waiting.push_back (&wobject);
      }

      glock.unlock ();

      int rc = wobject.wait (timeout_in_ms);

      {
        unique_lock wlock (m_waiting_mutex);
        m_waiting.remove (&wobject);
      }

      return rc;
    }

    void kvs_t::notify (key_type const &key, int events) const
    {
      onChange (key, events);

      shared_lock lock (m_waiting_mutex);
      std::list<waiting_t *>::const_iterator it = m_waiting.begin ();
      const std::list<waiting_t *>::const_iterator end = m_waiting.end ();

      while (it != end)
      {
        if ((*it)->key () == key)
        {
          (*it)->notify (events);
        }
        ++it;
      }
    }

    void kvs_t::purge_expired_keys () const
    {
      if (not m_expired_entries.empty ())
      {
        unique_lock lock (m_expired_entries_mutex);
        while (not m_expired_entries.empty ())
        {
          key_type key = m_expired_entries.front ();
          m_expired_entries.pop_front ();

          unique_lock values_lock (m_mutex);
          const_cast<kvs_t&>(*this).m_values.erase (key);
        }
      }
    }
  }
}
namespace gspc
{
  namespace kvs
  {
    service_t::service_t ()
      : m_kvs ("inproc://")
    {
      m_on_change_connection =
        m_kvs.onChange.connect (boost::bind ( &service_t::on_change
                                             , this
                                             , _1
                                             , _2
                                             )
                                );
      setup_rpc_handler ();
    }

    service_t::~service_t ()
    {
      m_on_change_connection.disconnect ();
    }

    gspc::kvs::api_t & service_t::api ()
    {
      return m_kvs;
    }

    void service_t::setup_rpc_handler ()
    {
      m_rpc_table ["put"]  = boost::bind (&service_t::rpc_put, this, _1);
      m_rpc_table ["get"]  = boost::bind (&service_t::rpc_get, this, _1);
      m_rpc_table ["get_regex"]  = boost::bind (&service_t::rpc_get_regex, this, _1);
      m_rpc_table ["del"]  = boost::bind (&service_t::rpc_del, this, _1);
      m_rpc_table ["del_regex"]  = boost::bind (&service_t::rpc_del_regex, this, _1);
      m_rpc_table ["set_ttl"]  = boost::bind (&service_t::rpc_set_ttl, this, _1);
      m_rpc_table ["set_ttl_regex"]  = boost::bind (&service_t::rpc_set_ttl_regex, this, _1);
      m_rpc_table ["push"]  = boost::bind (&service_t::rpc_push, this, _1);
      m_rpc_table ["wait"]  = boost::bind (&service_t::rpc_wait, this, _1);
      m_rpc_table ["try_pop"]  = boost::bind (&service_t::rpc_try_pop, this, _1);
      m_rpc_table ["counter_reset"]  = boost::bind (&service_t::rpc_counter_reset, this, _1);
      m_rpc_table ["counter_change"]  = boost::bind (&service_t::rpc_counter_change, this, _1);
      m_rpc_table ["help"] = boost::bind (&service_t::rpc_help, this, _1);
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_put (gspc::net::frame const &rqst)
    {
      fhg::util::parse::position_string pos (rqst.get_body ());

      std::list<std::pair<api_t::key_type, api_t::value_type> >
        values_to_put;

      while (not pos.end ())
      {
        if (*pos == '\n')
        {
          ++pos;
        }
        else
        {
          const api_t::key_type key =
            fhg::util::parse::require::plain_string (pos, ' ');
          api_t::value_type val = pnet::type::value::read (pos);
          values_to_put.push_back (std::make_pair (key, val));
        }
      }

      return encode_error_code (rqst, m_kvs.put (values_to_put));
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_get (gspc::net::frame const &rqst)
    {
      const api_t::key_type &key = rqst.get_body ();

      api_t::value_type val;

      return encode_error_code (rqst, m_kvs.get (key, val))
        << pnet::type::value::show (val) << "\n";
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_get_regex (gspc::net::frame const &rqst)
    {
      std::string const &regex = rqst.get_body ();

      typedef std::list<std::pair<api_t::key_type, api_t::value_type> > kv_list_t;
      kv_list_t values;
      int rc = m_kvs.get_regex (regex, values);

      gspc::net::frame rply = encode_error_code (rqst, rc);

      if (0 == rc)
      {
        kv_list_t::const_iterator it = values.begin ();
        const kv_list_t::const_iterator end = values.end ();

        for ( ; it != end ; ++it)
        {
          rply << it->first << " " << pnet::type::value::show (it->second)
               << "\n"
            ;
        }
      }

      return rply;
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_del (gspc::net::frame const &rqst)
    {
      return
        encode_error_code (rqst, m_kvs.del (rqst.get_body ()));
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_del_regex (gspc::net::frame const &rqst)
    {
      return
        encode_error_code (rqst, m_kvs.del_regex (rqst.get_body ()));
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_set_ttl (gspc::net::frame const &rqst)
    {
      fhg::util::parse::position_string pos (rqst.get_body ());

      if (pos.end ())
        return gspc::net::make::error_frame
          ( rqst
          , gspc::net::E_SERVICE_FAILED
          , "empty body"
          );

      int ttl = fhg::util::read_int (pos);
      fhg::util::parse::require::require (pos, ' ');

      api_t::key_type key =
        fhg::util::parse::require::plain_string (pos, '\n');

      return encode_error_code (rqst, m_kvs.set_ttl (key, ttl));
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_set_ttl_regex (gspc::net::frame const &rqst)
    {
      fhg::util::parse::position_string pos (rqst.get_body ());

      if (pos.end ())
        return gspc::net::make::error_frame
          ( rqst
          , gspc::net::E_SERVICE_FAILED
          , "empty body"
          );

      const int ttl = fhg::util::read_int (pos);
      fhg::util::parse::require::require (pos, ' ');

      const std::string regex =
        fhg::util::parse::require::string (pos);

      return encode_error_code (rqst, m_kvs.set_ttl_regex (regex, ttl));
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_push (gspc::net::frame const &rqst)
    {
      fhg::util::parse::position_string pos (rqst.get_body ());

      const api_t::key_type key =
        fhg::util::parse::require::plain_string (pos, ' ');
      const api_t::value_type val =
        pnet::type::value::read (pos);

      return encode_error_code (rqst, m_kvs.push (key, val));
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_wait (gspc::net::frame const &rqst)
    {
      fhg::util::parse::position_string pos (rqst.get_body ());

      if (pos.end ())
        return gspc::net::make::error_frame
          ( rqst
          , gspc::net::E_SERVICE_FAILED
          , "empty body"
          );

      int events = fhg::util::read_int (pos);
      fhg::util::parse::require::require (pos, ' ');

      api_t::key_type key =
        fhg::util::parse::require::plain_string (pos, '\n');

      int rc = m_kvs.wait (key, events, 0);
      if (rc > 0)
      {
        return encode_error_code (rqst, rc);
      }
      else
      {
        waiting_t wobject;
        wobject.rqst = rqst;
        wobject.key = key;
        wobject.mask = events;
        wobject.events = 0;

        boost::unique_lock<boost::shared_mutex> lock (m_waiting_mtx);
        m_waiting.push_back (wobject);

        return boost::none;
      }
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_try_pop (gspc::net::frame const &rqst)
    {
      api_t::value_type val;
      int rc = m_kvs.try_pop (rqst.get_body (), val);
      gspc::net::frame rply = encode_error_code (rqst, rc);
      if (0 == rc)
      {
        rply << pnet::type::value::show (val) << "\n";
      }
      return rply;
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_counter_reset (gspc::net::frame const &rqst)
    {
      fhg::util::parse::position_string pos (rqst.get_body ());

      if (pos.end ())
        return gspc::net::make::error_frame
          ( rqst
          , gspc::net::E_SERVICE_FAILED
          , "empty body"
          );

      int val = fhg::util::read_int (pos);
      fhg::util::parse::require::require (pos, ' ');

      api_t::key_type key =
        fhg::util::parse::require::plain_string (pos, '\n');

      return encode_error_code (rqst, m_kvs.counter_reset (key, val));
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_counter_change (gspc::net::frame const &rqst)
    {
      fhg::util::parse::position_string pos (rqst.get_body ());

      if (pos.end ())
        return gspc::net::make::error_frame
          ( rqst
          , gspc::net::E_SERVICE_FAILED
          , "empty body"
          );

      int delta = fhg::util::read_int (pos);
      fhg::util::parse::require::require (pos, ' ');

      api_t::key_type key =
        fhg::util::parse::require::plain_string (pos, '\n');

      int val;

      return encode_error_code (rqst, m_kvs.counter_change (key, val, delta))
        << val << "\n";
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_help (gspc::net::frame const &rqst)
    {
      gspc::net::frame rply = gspc::net::make::reply_frame (rqst);
      rply << "available RPC:" << "\n";
      rpc_table_t::const_iterator it = m_rpc_table.begin ();
      const rpc_table_t::const_iterator end = m_rpc_table.end ();

      for ( ; it != end ; ++it)
      {
        rply << "   " << it->first << "\n";
      }

      return rply;
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_invalid (gspc::net::frame const &rqst)
    {
      return gspc::net::make::error_frame
        ( rqst
        , gspc::net::E_SERVICE_NO_SUCH_RPC
        , "kvs"
        );
    }

    gspc::net::frame
    service_t::encode_error_code ( gspc::net::frame const &rqst
                                 , int rc
                                 )
    {
      return gspc::net::make::reply_frame (rqst) << rc << "\n";
    }

    void service_t::operator () ( std::string const &dst
                                , gspc::net::frame const &rqst
                                , gspc::net::user_ptr user
                                )
    {
      rpc_table_t::const_iterator it = m_rpc_table.find (dst);
      if (it == m_rpc_table.end ())
      {
        gspc::net::frame rply = *rpc_invalid (rqst);
        user->deliver (rply);
      }
      else
      {
        boost::optional<gspc::net::frame> rply = it->second (rqst);
        if (rply)
          user->deliver (*rply);
      }
    }

    void service_t::on_change (api_t::key_type const &key, int events)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_waiting_mtx);
      waiting_list_t::iterator it = m_waiting.begin ();
      const waiting_list_t::iterator end = m_waiting.end ();

      while (it != end)
      {
        if (it->key == key && it->mask & events)
        {
          gspc::net::frame rply = encode_error_code (it->rqst, events);
          gspc::net::server::default_queue_manager ().deliver (rply);

          it = m_waiting.erase (it);
        }
        else
        {
          ++it;
        }
      }
    }
  }
}
