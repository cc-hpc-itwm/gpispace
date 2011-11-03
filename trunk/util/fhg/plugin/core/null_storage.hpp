#ifndef FHG_PLUGIN_NULL_STORAGE_HPP
#define FHG_PLUGIN_NULL_STORAGE_HPP 1

#include <boost/unordered_map.hpp>
#include <boost/thread.hpp>

#include <fhg/plugin/storage.hpp>

namespace fhg
{
  namespace plugin
  {
    namespace core
    {
      class NullStorage : public fhg::plugin::Storage
      {
        typedef boost::unordered_map<std::string, std::string> value_map_t;
        typedef boost::unordered_map<std::string, NullStorage*> store_map_t;

        typedef boost::condition_variable_any condition_type;
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;
      public:
        ~NullStorage () {}

        int add_storage (std::string const &key);
        Storage *get_storage (std::string const &key) const;
        int del_storage (std::string const &key);

        int remove (std::string const &key);
        int commit ();
        int flush ();
      protected:
        int write (std::string const &key, std::string const &value);
        int read (std::string const &key, std::string &value) const;
      private:
        mutable mutex_type m_mutex;

        value_map_t m_values;
        store_map_t m_stores;
      };
    }
  }
}

#endif
