//#define BOOST_FILESYSTEM_NO_DEPRECATED

#include <errno.h>

#include "file_storage.hpp"

namespace fhg
{
  namespace plugin
  {
    namespace core
    {
      namespace detail
      {
        FileStorage* create_storage (FileStorage::path_t const &)
        {
          return 0;
        }
      }

      FileStorage::FileStorage(path_t const & path)
        : m_path (path)
      {}

      FileStorage::~FileStorage()
      {
        flush();
      }

      bool FileStorage::validate(std::string const &key)
      {
        if (key.empty()) return false;
        if (key.find("/") != std::string::npos) return false;
        if (key.find(" ") != std::string::npos) return false;
        if (key == "." || key == "..") return false;

        return true;
      }

      int FileStorage::restore ()
      {
        restore_storages();
        restore_values();
        return 0;
      }

      int FileStorage::add_storage(std::string const &s)
      {
        if (not validate(s))
          return -EINVAL;

        lock_type lock (m_mutex);

        path_t p (m_path / s);

        // create directory

        // create file storage with that directory

        return -EPERM;
      }

      FileStorage* FileStorage::get_storage(std::string const&) const
      {
        return 0;
      }

      int FileStorage::del_storage(std::string const &)
      {
        return -ESRCH;
      }

      int FileStorage::write (std::string const &key, std::string const &value)
      {
        if (not validate(key))
          return -EINVAL;

        value_t val (value);

        path_t p(m_path / key);

        {
          lock_type lock(m_mutex);
          if (m_stores.find(key) != m_stores.end())
            return -EPERM;
          if (m_values.find(key) == m_values.end())
          {
            m_values[key] = val;
          }
        }

        return 0;
      }

      int FileStorage::read (std::string const &key, std::string &value)
      {
        if (not validate(key))
        {
          return -EINVAL;
        }

        lock_type lock(m_mutex);
        if (m_stores.find(key) != m_stores.end())
          return -EPERM;
        value_map_t::iterator v_it(m_values.find(key));
        if (v_it == m_values.end())
        {
          v_it = restore_value(key);
          if (v_it == m_values.end())
          {
            return -ESRCH;
          }
        }
        value = v_it->second.value;
        return 0;
      }

      int FileStorage::remove (std::string const &key)
      {
        return 0;
      }

      int FileStorage::flush ()
      {
        return 0;
      }

      int FileStorage::commit ()
      {
        lock_type lock (m_mutex);

        // write transaction to file system

        // merge transaction with values

        return 0;
      }

      // restore the layout discovered from the filesystem
      void FileStorage::restore_storages()
      {
      }

      void FileStorage::restore_values()
      {
      }

      FileStorage::storage_map_t::iterator
      FileStorage::restore_storage(path_t const & path)
      {
        FileStorage *child = detail::create_storage (m_path / path);
        if (child)
        {
          m_stores[path.string()] = child;
          child->restore();
          return m_stores.find(path.string());
        }
        return m_stores.end();
      }

      FileStorage::value_map_t::iterator
      FileStorage::restore_value(path_t const & path)
      {
        return m_values.end();
      }
    }
  }
}
