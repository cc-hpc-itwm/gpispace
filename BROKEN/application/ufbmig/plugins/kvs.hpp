#ifndef KVS_PLUGIN_HPP
#define KVS_PLUGIN_HPP 1

#include <string>
#include <map>
#include <boost/lexical_cast.hpp>

namespace kvs
{
  class KeyValueStore
  {
  public:
    typedef std::string key_type;
    typedef std::string value_type;
    typedef std::map<key_type, value_type> key_value_map_type;

    virtual ~KeyValueStore() {}

    virtual value_type get(key_type const & k, value_type const &dflt) const = 0;
    virtual void       put(key_type const & k, value_type const &value) = 0;
    virtual void       del(key_type const & k) = 0;
    virtual int        inc(key_type const & k, int step = 1) = 0;
    virtual key_value_map_type list () const = 0;
    virtual key_value_map_type list (key_type const &prefix) const = 0;

    template <typename T>
    T get(key_type const & k, value_type const & dflt) const
    {
      value_type v (this->get(k, dflt));
      try
      {
        return boost::lexical_cast<T>(v);
      }
      catch (boost::bad_lexical_cast const &)
      {
        return dflt;
      }
    }

    template <typename T>
    T get(key_type const & k, T const & dflt) const
    {
      value_type v (this->get(k, boost::lexical_cast<std::string>(dflt)));
      try
      {
        return boost::lexical_cast<T>(v);
      }
      catch (boost::bad_lexical_cast const &)
      {
        return dflt;
      }
    }

    template <typename T>
    void put (key_type const &k, T const & v)
    {
      return this->put(k, boost::lexical_cast<std::string>(v));
    }
  };
}

#endif
