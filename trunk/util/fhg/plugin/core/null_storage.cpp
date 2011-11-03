#include <errno.h>

#include "null_storage.hpp"

namespace fhg
{
  namespace plugin
  {
    namespace core
    {
      int NullStorage::add_storage (std::string const &key)
      {
        lock_type lock (m_mutex);

        if (m_values.find (key) != m_values.end())
          return -ENOTDIR;
        else
        {
          if (m_stores.find(key) == m_stores.end())
            m_stores[key] = new NullStorage();
        }
        return 0;
      }

      Storage *NullStorage::get_storage (std::string const &key) const
      {
        lock_type lock (m_mutex);
        store_map_t::const_iterator it (m_stores.find(key));
        if (it != m_stores.end())
          return it->second;
        else
          return 0;
      }

      int NullStorage::del_storage (std::string const &key)
      {
        lock_type lock (m_mutex);
        store_map_t::const_iterator it (m_stores.find(key));
        if (it != m_stores.end())
        {
          delete it->second;
          m_stores.erase (it);
          return 0;
        }
        else
        {
          return -ENOENT;
        }
      }

      int NullStorage::remove (std::string const &key)
      {
        lock_type lock (m_mutex);
        if (m_stores.find(key) != m_stores.end())
        {
          return del_storage(key);
        }
        else
        {
          if (m_values.erase (key) < 1)
          {
            return -ENOENT;
          }
          else
          {
            return 0;
          }
        }
      }

      int NullStorage::commit ()
      {
        return 0;
      }

      int NullStorage::flush ()
      {
        return 0;
      }

      int NullStorage::write (std::string const &key, std::string const &value)
      {
        lock_type lock (m_mutex);

        if (m_stores.find(key) != m_stores.end())
        {
          return -EISDIR;
        }
        else
        {
          m_values[key] = value;
          return 0;
        }
      }

      int NullStorage::read (std::string const &key, std::string &value) const
      {
        lock_type lock (m_mutex);

        if (m_stores.find(key) != m_stores.end())
        {
          return -EISDIR;
        }
        else
        {
          value_map_t::const_iterator it (m_values.find(key));
          if (it == m_values.end())
          {
            return -ENOENT;
          }
          else
          {
            value = it->second;
            return 0;
          }
        }
      }
    }
  }
}
