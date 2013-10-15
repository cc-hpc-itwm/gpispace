#include "kvs_impl.hpp"
#include <boost/thread/locks.hpp>

typedef boost::shared_lock<boost::shared_mutex> shared_lock;
typedef boost::unique_lock<boost::shared_mutex> unique_lock;

namespace gspc
{
  namespace kvs
  {
    kvs_t::~kvs_t () {}

    int kvs_t::put (key_type const &key, value_type const &val)
    {
      {
        unique_lock lock (m_mutex);
        m_values.insert (std::make_pair (key, val));
      }

      onChange (key);

      return 0;
    }

    int kvs_t::lookup (key_type const &key, value_type &val) const
    {
      std::map<key_type, value_type>::const_iterator it = m_values.find (key);
      if (it == m_values.end ())
        return -ESRCH;

      val = it->second;

      return 0;
    }

    int kvs_t::lookup (key_type const &key, value_type &val)
    {
      return const_cast<kvs_t const&>(*this).lookup (key, val);
    }

    int kvs_t::get (key_type const &key, value_type &val) const
    {
      shared_lock lock (m_mutex);

      return lookup (key, val);
    }

    int kvs_t::del (key_type const &key)
    {
      {
        unique_lock lock (m_mutex);

        std::map<key_type, value_type>::iterator it = m_values.find (key);
        if (it == m_values.end ())
          return -ESRCH;

        m_values.erase (it);
      }

      onChange (key);

      return 0;
    }

    int kvs_t::push (key_type const &key, value_type const &val)
    {
      {
        unique_lock lock (m_mutex);

        std::map<key_type, value_type>::iterator it = m_values.find (key);
        if (it == m_values.end ())
        {
          std::list<value_type> alist;

          it = m_values.insert (std::make_pair (key, alist)).first;
        }

        std::list<value_type> *alist =
          boost::get<std::list<value_type> >(&it->second);

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
      {
        unique_lock lock (m_mutex);

        std::map<key_type, value_type>::iterator it = m_values.find (key);
        if (it == m_values.end ())
        {
          return -ESRCH;
        }

        std::list<value_type> *alist =
          boost::get<std::list<value_type> >(&it->second);

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
      {
        unique_lock lock (m_mutex);

        std::map<key_type, value_type>::iterator it = m_values.find (key);
        if (it == m_values.end ())
        {
          m_values.insert (std::make_pair (key, val));
        }
        else
        {
          it->second = val;
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
      {
        unique_lock lock (m_mutex);

        std::map<key_type, value_type>::iterator it = m_values.find (key);
        if (it == m_values.end ())
        {
          return -ESRCH;
        }

        int *cur = boost::get<int>(&it->second);
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
  }
}
