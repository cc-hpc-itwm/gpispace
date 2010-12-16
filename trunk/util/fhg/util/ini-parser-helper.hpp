#ifndef FHG_UTIL_INI_PARSER_HELPER_HPP
#define FHG_UTIL_INI_PARSER_HELPER_HPP 1

#include <string>

namespace fhg
{
namespace util
{
namespace ini
{
  template <typename MapType>
  struct parse_into_map_t
  {
    typedef MapType map_type;

    int operator () ( std::string const & sec
                    , std::string const * secid
                    , std::string const & key
                    , std::string const & val
                    )
    {
      return handle (sec,secid,key,val);
    }

    int handle ( std::string const & sec
               , std::string const * secid
               , std::string const & key
               , std::string const & val
               )
    {
      std::string k (secid ? (sec + "." + *secid) : sec);
      k += ".";
      k += key;
      entries[k] = val;
      return 0;
    }

    std::string get ( std::string const & sec
                    , std::string const & key
                    , std::string const & def
                    )
    {
      try
      {
        std::string k (sec + "." + key);
        return entries.at(k);
      }
      catch (std::exception const &)
      {
        return def;
      }
    }

    std::string get ( std::string const & sec
                    , std::string const & sec_id
                    , std::string const & key
                    , std::string const & def
                    )
    {
      return get (sec + "." + sec_id, key, def);
    }

    map_type entries;
  };
}
}
}

#endif
