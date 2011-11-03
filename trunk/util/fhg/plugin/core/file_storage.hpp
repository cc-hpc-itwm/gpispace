#ifndef FHG_PLUGIN_CORE_FILE_STORAGE_HPP
#define FHG_PLUGIN_CORE_FILE_STORAGE_HPP 1

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <list>

#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#include <fhg/plugin/storage.hpp>

namespace fhg
{
  namespace plugin
  {
    namespace core
    {
      class FileStorage : public fhg::plugin::Storage
      {
      public:
        typedef boost::filesystem::path path_t;
      private:
        struct value_t
        {
          value_t ()
            : modified(false)
          {}

          explicit value_t (std::string const &v)
            : modified (false)
            , value (v)
          {}

          bool modified;
          std::string value;
        };

        typedef std::string key_t;
        typedef boost::unordered_map<key_t, value_t> value_map_t;

        typedef boost::condition_variable_any condition_type;
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        typedef boost::unordered_map<key_t, FileStorage*> store_map_t;
      public:
        ~FileStorage();

        explicit
        FileStorage (path_t const & path, int flags = O_CREAT, int mode = S_IRWXU);

        int restore ();

        int add_storage(std::string const &key);
        Storage *get_storage(std::string const & key) const;
        int del_storage(std::string const &key);

        int remove (std::string const &key);
        int commit ();
        int flush ();
      protected:
        int write (std::string const &key, std::string const &value);
        int read (std::string const &key, std::string &value) const;
      private:
        // restore the layout discovered from the filesystem
        void restore_storages();
        void restore_values();
        store_map_t::iterator restore_storage(path_t const & path);
        value_map_t::iterator restore_value(path_t const & path);

        mutable mutex_type m_mutex;
        path_t m_path;
        int m_mode;
        int m_flags;
        store_map_t m_stores;
        value_map_t m_values;
      };
    }
  }
}

#endif
