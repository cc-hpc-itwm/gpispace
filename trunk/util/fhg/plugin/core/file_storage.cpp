//#define BOOST_FILESYSTEM_NO_DEPRECATED

#include <errno.h>

#include <fhglog/minimal.hpp>

#include <boost/filesystem/fstream.hpp>
#include <boost/system/error_code.hpp>

#include "file_storage.hpp"

namespace fs = boost::filesystem;

#define FILE_STORAGE_VERSION 1

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

      FileStorage::FileStorage(path_t const & path, int flags, int mode)
        : m_path (fs::absolute(path))
        , m_mode (mode)
      {
        if (fs::exists(m_path))
        {
          check_storage_path (m_path, mode, FILE_STORAGE_VERSION);
        }
        else if (flags & O_CREAT)
        {
          init_storage_path (m_path, mode, FILE_STORAGE_VERSION);
        }
        else
        {
          throw fs::filesystem_error
            ( "no such file or directory"
            , path
            , boost::system::errc::make_error_code(boost::system::errc::no_such_file_or_directory)
            );
        }

        lock_storage_path();
      }

      void FileStorage::lock_storage_path ()
      {
        path_t lock_file (m_path / ".lock");
        if (fs::exists(lock_file))
        {
          MLOG(WARN, "lock file still exists!");
        }

        fs::ofstream s (lock_file);
        if (s)
        {
          s << getpid() << std::endl;
        }
        else
        {
          throw fs::filesystem_error
            ( "could not lock directory"
            , m_path
            , boost::system::errc::make_error_code(boost::system::errc::permission_denied)
            );
        }
      }

      void FileStorage::unlock_storage_path ()
      {
        path_t lock_file (m_path / ".lock");
        if (fs::exists(lock_file))
        {
          fs::ifstream s (lock_file);
          int pid (0);
          s >> pid;

          if (pid != getpid())
          {
            MLOG(WARN, "i cannot unlock the lock file: not owner!");
          }
          else
          {
            boost::system::error_code ec;
            fs::remove (m_path / ".lock", ec);
          }
        }
      }

      void FileStorage::check_storage_path (path_t const &path, int mode, const int version)
      {
        if (! fs::is_directory(path))
        {
          throw fs::filesystem_error
            ( "not a directory"
            , path
            , boost::system::errc::make_error_code(boost::system::errc::not_a_directory)
            );
        }
        else
        {
          fs::ifstream file (path / ".storage");
          if (file)
          {
            int found_version (0);
            file >> found_version;
            if (version != found_version)
            {
              MLOG(WARN, "version mismatch of storage directory: expected := " << version << " found := " << found_version);
            }
          }
          else
          {
            MLOG(WARN, "path is not a valid storage directory: " << path);
            init_storage_path (path, mode, version);
          }
        }
      }

      void FileStorage::init_storage_path (path_t const &path, int mode, const int version)
      {
        fs::create_directories (path);
        // this only works on UNIX
        chmod (path.string().c_str(), mode);

        fs::ofstream file (m_path / ".storage");
        if (file)
        {
          file << version << std::endl;
        }
      }

      FileStorage::~FileStorage()
      {
        flush();
        for ( store_map_t::iterator start (m_stores.begin()), end(m_stores.end())
            ; start != end
            ; ++start
            )
        {
          delete start->second;
        }
        m_stores.clear();
        m_values.clear();

        unlock_storage_path();
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
        if (fs::exists(p) && fs::is_regular_file(p))
        {
          return -ENOTDIR;
        }
        else
        {
          if (m_stores.find(s) == m_stores.end())
          {
            try
            {
              m_stores[s] = new FileStorage(p, m_flags | O_CREAT, m_mode);
            }
            catch (std::exception const & ex)
            {
              return -EPERM;
            }
          }
          return 0;
        }
      }

      Storage* FileStorage::get_storage(std::string const&k) const
      {
        if (not validate(k))
          return 0;
        lock_type lock (m_mutex);
        store_map_t::const_iterator it (m_stores.find(k));
        if (it == m_stores.end())
        {
          return 0;
        }
        else
        {
          return it->second;
        }
      }

      int FileStorage::del_storage(std::string const &k)
      {
        if (not validate(k))
          return -EINVAL;
        lock_type lock (m_mutex);
        store_map_t::iterator it (m_stores.find(k));
        if (it == m_stores.end())
        {
          return -ENOENT;
        }
        else
        {
          delete it->second;
          m_stores.erase (it);
          return 0;
        }
      }

      int FileStorage::write (std::string const &key, std::string const &value)
      {
        if (not validate(key))
          return -EINVAL;

        lock_type lock(m_mutex);

        path_t p(m_path / key);

        if (fs::exists(p) && fs::is_directory(p))
        {
          return -EISDIR;
        }
        else
        {
          fs::ofstream s (p);
          if (s)
          {
            s << value;
            return 0;
          }
          else
          {
            return -EPERM;
          }
        }
      }

      int FileStorage::read (std::string const &key, std::string &value) const
      {
        if (not validate(key))
          return -EINVAL;

        lock_type lock(m_mutex);

        path_t p(m_path / key);

        if (fs::exists(p) && fs::is_directory(p))
        {
          return -EISDIR;
        }
        else
        {
          fs::ifstream s (p);
          if (s)
          {
            std::stringstream sstr;
            s >> std::noskipws >> sstr.rdbuf();
            value = sstr.str();
            return 0;
          }
          else
          {
            return -ENOENT;
          }
        }
      }

      int FileStorage::remove (std::string const &key)
      {
        if (not validate(key))
          return -EINVAL;

        path_t p(m_path / key);

        if (fs::is_directory(p))
        {
          // TODO: remove storage associated with key
          return -EISDIR;
        }
        else
        {
          if (fs::remove(p))
          {
            return 0;
          }
          else
          {
            return -ENOENT;
          }
        }
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

      FileStorage::store_map_t::iterator
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
