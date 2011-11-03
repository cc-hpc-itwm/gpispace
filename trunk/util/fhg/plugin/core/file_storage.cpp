//#define BOOST_FILESYSTEM_NO_DEPRECATED

#include <errno.h>

#include <boost/filesystem/fstream.hpp>
#include <boost/system/error_code.hpp>

#include "file_storage.hpp"

namespace fs = boost::filesystem;

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
          if (! fs::is_directory(m_path))
          {
            throw fs::filesystem_error
              ( "not a directory"
              , path
              , boost::system::errc::make_error_code(boost::system::errc::not_a_directory)
              );
          }
        }
        else
        {
          if (flags & O_CREAT)
          {
            fs::create_directories (m_path);
            // this only works on UNIX
            chmod (m_path.string().c_str(), mode);

            fs::ofstream file (m_path / ".storage");
            if (file)
            {
              file << time(0);
            }
          }
          else
          {
            throw fs::filesystem_error
              ( "no such file or directory"
              , path
              , boost::system::errc::make_error_code(boost::system::errc::no_such_file_or_directory)
              );
          }
        }

        fs::ofstream lock_file (m_path / ".lock");
        if (lock_file)
        {
          lock_file << getpid();
        }
        else
        {
          throw fs::filesystem_error
            ( "could not lock directory"
            , path
            , boost::system::errc::make_error_code(boost::system::errc::permission_denied)
            );
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

        boost::system::error_code ec;
        fs::remove (m_path / ".lock", ec);
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
          s << value;
          return 0;
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
