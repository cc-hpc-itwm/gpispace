#include "kvs.hpp"

#include <map>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

typedef kvs::KeyValueStore::key_value_map_type key_value_map_t;

class KeyValueStorePlugin : FHG_PLUGIN
                          , public kvs::KeyValueStore
{
public:
  typedef boost::recursive_mutex mutex_type;
  typedef boost::condition_variable condition_type;
  typedef boost::unique_lock<mutex_type> lock_type;

  FHG_PLUGIN_START()
  {
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    FHG_PLUGIN_STOPPED();
  }

  value_type get(key_type const & k, value_type const &dflt) const
  {
    lock_type lock (m_mutex);

    key_value_map_t::const_iterator entry = m_store.find (k);
    if (entry == m_store.end ())
      return dflt;
    else
      return entry->second;
  }

  void       put(key_type const & k, value_type const &value)
  {
    lock_type lock (m_mutex);

    m_store [k] = value;
  }

  void       del(key_type const & k)
  {
    lock_type lock (m_mutex);

    m_store.erase (k);
  }

  key_value_map_type list () const
  {
    lock_type lock (m_mutex);
    return m_store;
  }

  key_value_map_type list (key_type const &prefix) const
  {
    key_value_map_type values;
    lock_type lock (m_mutex);

    key_value_map_t::const_iterator entry = m_store.begin ();
    const key_value_map_t::const_iterator end = m_store.end ();

    while (entry != end)
    {
      if (entry->first.find (prefix) == 0)
      {
        values.insert (*entry);
      }

      ++entry;
    }

    return values;
  }

  int        inc(key_type const & k, int step)
  {
    lock_type lock (m_mutex);

    int counter;

    key_value_map_t::iterator entry = m_store.find (k);
    if (entry != m_store.end ())
    {
      counter = boost::lexical_cast<int> (entry->second);
    }
    else
    {
      counter = 0;
    }

    counter += step;
    this->put (k, boost::lexical_cast<std::string> (counter));
    return step;
  }

private:
  mutable mutex_type m_mutex;

  key_value_map_t m_store;
};


EXPORT_FHG_PLUGIN( kvs
                 , KeyValueStorePlugin
                 , "kvs"
                 , "provides access to a key value store"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.2"
                 , "NA"
                 , ""
                 , ""
                 );
