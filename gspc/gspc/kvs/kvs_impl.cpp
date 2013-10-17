#include "kvs_impl.hpp"

#include <algorithm>
#include <boost/regex.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/locks.hpp>

typedef boost::shared_lock<boost::shared_mutex> shared_lock;
typedef boost::unique_lock<boost::shared_mutex> unique_lock;

namespace gspc
{
  namespace kvs
  {
    kvs_t::~kvs_t () {}

    int kvs_t::get (key_type const &key, value_type &val) const
    {
      purge_expired_keys ();

      shared_lock lock (m_mutex);

      value_map_t::const_iterator it = m_values.find (key);
      if (it == m_values.end ())
        return -ESRCH;

      if (it->second.is_expired ())
      {
        unique_lock expired_lock (m_expired_entries_mutex);
        m_expired_entries.push_back (it->first);
        return -ESRCH;
      }
      else
      {
        val = it->second.value;
      }

      return 0;
    }

    int kvs_t::put (key_type const &key, value_type const &val)
    {
      purge_expired_keys ();

      {
        unique_lock lock (m_mutex);
        m_values.insert (std::make_pair (key, entry_t (val)));
      }

      onChange (key);

      return 0;
    }

    int kvs_t::put (std::list<std::pair<key_type, value_type> > const &values)
    {
      purge_expired_keys ();

      typedef std::pair<key_type, value_type> kv_pair_t;
      BOOST_FOREACH (kv_pair_t const &kp, values)
      {
        put (kp.first, kp.second);
      }

      return 0;
    }

    int kvs_t::get_regex ( std::string const &r
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
          if (it->second.is_expired ())
          {
            unique_lock expired_lock (m_expired_entries_mutex);
            m_expired_entries.push_back (it->first);
          }
          else
          {
            values.push_back (std::make_pair (it->first, it->second.value));
          }
        }

        ++it;
      }

      return 0;
    }

    int kvs_t::del (key_type const &key)
    {
      purge_expired_keys ();

      {
        unique_lock lock (m_mutex);

        value_map_t::iterator it = m_values.find (key);
        if (it == m_values.end ())
          return -ESRCH;

        m_values.erase (it);
      }

      onChange (key);

      return 0;
    }

    int kvs_t::del_regex (std::string const &r)
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
        onChange (key);
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

    int kvs_t::set_ttl (key_type const &key, int ttl)
    {
      purge_expired_keys ();

      unique_lock lock (m_mutex);

      value_map_t::iterator it = m_values.find (key);
      if (it == m_values.end ())
        return -ESRCH;

      it->second.expires_in (ttl);

      return 0;
    }

    int kvs_t::set_ttl_regex (std::string const &r, int ttl)
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
          it->second.expires_in (ttl);
        }

        ++it;
      }

      return 0;
    }

    int kvs_t::push (key_type const &key, value_type const &val)
    {
      purge_expired_keys ();

      {
        unique_lock lock (m_mutex);

        value_map_t::iterator it = m_values.find (key);
        if (it == m_values.end ())
        {
          std::list<value_type> alist;

          it = m_values.insert (std::make_pair (key, entry_t (alist))).first;
        }

        std::list<value_type> *alist =
          boost::get<std::list<value_type> >(&it->second.value);

        if (alist)
        {
          alist->push_back (val);
        }
        else
        {
          return -EINVAL;
        }
      }

      onChange (key);

      return 0;
    }

    int kvs_t::try_pop (key_type const &key, value_type &val)
    {
      purge_expired_keys ();

      {
        unique_lock lock (m_mutex);

        value_map_t::iterator it = m_values.find (key);
        if (it == m_values.end ())
        {
          return -ESRCH;
        }

        std::list<value_type> *alist =
          boost::get<std::list<value_type> >(&it->second.value);

        if (alist)
        {
          if (alist->empty ())
          {
            return -EAGAIN;
          }
          else
          {
            val = alist->front (); alist->pop_front ();
          }
        }
        else
        {
          return -EINVAL;
        }
      }

      onChange (key);

      return 0;
    }

    int kvs_t::counter_reset (key_type const &key, int val)
    {
      purge_expired_keys ();

      {
        unique_lock lock (m_mutex);

        value_map_t::iterator it = m_values.find (key);
        if (it == m_values.end ())
        {
          m_values.insert (it, std::make_pair (key, entry_t (val)));
        }
        else
        {
          it->second.value = val;
        }
      }

      onChange (key);

      return 0;
    }

    int kvs_t::counter_increment (key_type const &key, int &val)
    {
      return counter_increment (key, val, 1);
    }

    int kvs_t::counter_increment (key_type const &key, int &val, int delta)
    {
      purge_expired_keys ();

      {
        unique_lock lock (m_mutex);

        value_map_t::iterator it = m_values.find (key);
        if (it == m_values.end ())
        {
          return -ESRCH;
        }

        int *cur = boost::get<int>(&it->second.value);
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

      onChange (key);

      return 0;
    }

    int kvs_t::counter_decrement (key_type const &key, int &val)
    {
      return counter_decrement (key, val, 1);
    }

    int kvs_t::counter_decrement (key_type const &key, int &val, int delta)
    {
      return counter_increment (key, val, -delta);
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
