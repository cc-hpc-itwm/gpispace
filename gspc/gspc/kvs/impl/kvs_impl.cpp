#include "kvs_impl.hpp"

#include <algorithm>
#include <boost/regex.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/locks.hpp>

#include <iomanip>

typedef boost::shared_lock<boost::shared_mutex> shared_lock;
typedef boost::unique_lock<boost::shared_mutex> unique_lock;

namespace gspc
{
  namespace kvs
  {
    kvs_t::kvs_t () {}
    kvs_t::kvs_t (std::string const &) {}

    kvs_t::~kvs_t () {}

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
        notify (kp.first, E_PUT);
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
      value_changed.notify_all ();
    }

    kvs_t::value_type & kvs_t::entry_t::get_value ()
    {
      return value;
    }

    kvs_t::value_type const & kvs_t::entry_t::get_value () const
    {
      return value;
    }

    int kvs_t::entry_t::pop (value_type &val)
    {
      boost::unique_lock<boost::recursive_mutex> lock (mutex);

      int rc = is_value_available ();

      while (rc == -EAGAIN)
      {
        value_changed.wait (lock);
        rc = is_value_available ();
      }

      if (rc == 0)
      {
        return try_pop (val);
      }
      else
      {
        return rc;
      }
    }

    int kvs_t::entry_t::pop (value_type &val, int timeout)
    {
      if (-1 == timeout)
        return this->pop (val);

      boost::system_time const deadline =
        boost::get_system_time() + boost::posix_time::milliseconds (timeout);

      boost::unique_lock<boost::recursive_mutex> lock (mutex);

      int rc = is_value_available ();

      while (rc == -EAGAIN)
      {
        if (not value_changed.timed_wait (lock, deadline))
        {
          rc = is_value_available ();
          if (rc == -EAGAIN)
            return -ETIME;
        }
        else
        {
          rc = is_value_available ();
        }
      }

      if (rc == 0)
      {
        return try_pop (val);
      }
      else
      {
        return rc;
      }
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
        value_changed.notify_one ();
        return 0;
      }
      else
      {
        return -EINVAL;
      }
    }

    int kvs_t::entry_t::is_value_available () const
    {
      const std::list<value_type> *alist =
        boost::get<std::list<value_type> >(&value);

      if (alist)
      {
        if (not alist->empty ())
          return 0;
        else
          return -EAGAIN;
      }

      return -EINVAL;
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

      notify (key, E_PUSH);

      return 0;
    }

    int kvs_t::do_pop (key_type const &key, value_type &val, int timeout)
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

        entry_ptr_t e = it->second;
        lock.unlock ();

        int rc = e->pop (val, timeout);
        if (rc != 0)
          return rc;
      }

      notify (key, E_POP);

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

      notify (key, E_PUT);

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
          return -ENOKEY;
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

      notify (key, E_PUT);

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
      waiting_t wobject (key, mask);

      {
        unique_lock lock (m_waiting_mutex);
        m_waiting.push_back (&wobject);
      }

      int rc = wobject.wait (timeout_in_ms);

      {
        unique_lock lock (m_waiting_mutex);
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
