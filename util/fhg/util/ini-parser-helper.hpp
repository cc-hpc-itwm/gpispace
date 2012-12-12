#ifndef FHG_UTIL_INI_PARSER_HELPER_HPP
#define FHG_UTIL_INI_PARSER_HELPER_HPP 1

#include <string>
#include <fhg/util/ini-parser.hpp>

namespace fhg
{
namespace util
{
namespace ini
{
  namespace detail
  {
    inline void unflatten ( std::string const & full_key, key_desc_t & key )
    {
      std::string::size_type sec_pos (full_key.find_first_of ('.'));
      if (sec_pos == std::string::npos)
      {
        throw std::runtime_error ("invalid key: " + full_key);
      }

      key.sec = full_key.substr(0, sec_pos);
      if (key.sec.empty())
      {
        throw std::runtime_error ("key does not contain a section: " + full_key);
      }

      std::string::size_type key_pos (full_key.find_last_of ('.'));
      if (key_pos == std::string::npos)
      {
        throw std::runtime_error ("invalid key: " + full_key);
      }

      key.key = full_key.substr (key_pos+1);
      if (key.key.empty())
      {
        throw std::runtime_error ("empty key: " + full_key);
      }

      if (sec_pos != key_pos)
      {
        key.id = boost::optional<std::string>(full_key.substr (sec_pos+1, key_pos - sec_pos - 1));
      }
      else
      {
        key.id = boost::optional<std::string>();
      }
    }
    inline key_desc_t unflatten ( std::string const & key )
    {
      key_desc_t k;
      unflatten(key, k);
      return k;
    }

    inline void flatten ( key_desc_t const & key, std::string & full_key )
    {
      full_key = key.sec;
      if (key.id)
        full_key += "." + *key.id;
      full_key += "." + key.key;
    }

    inline std::string flatten ( key_desc_t const & key )
    {
      std::string k;
      flatten(key, k);
      return k;
    }
  }

  namespace parser
  {
    template <typename MapType>
    struct flat_map_parser_t
    {
      typedef MapType entries_t;

      int operator () (key_desc_t const & key, std::string const & val)
      {
        return handle (key,val);
      }

      int handle (key_desc_t const & key, std::string const & val)
      {
          // TODO make this configurable -> i.e. hand in state somehow
        const bool W_already_defined (false);

        std::string flat_key (detail::flatten (key));
        if (W_already_defined && entries.find (flat_key) != entries.end())
        {
          std::cerr << "W: key already defined: " << flat_key << std::endl;
          return 1; // already there
        }
        else
        {
          entries [flat_key] = val;
          return 0;
        }
      }

      std::string get ( key_desc_t const & key, std::string const & def ) const
      {
        return get( detail::flatten( key ), def );
      }
      std::string get ( std::string const & key, std::string const & def ) const
      {
        try
        {
          return entries.at( key );
        }
        catch (std::exception const &)
        {
          return def;
        }
      }

      bool has_key (key_desc_t const & key) const
      {
        return has_key (detail::flatten (key));
      }

      bool has_key (std::string const &key) const
      {
        return entries.find (key) != entries.end ();
      }

      void put ( std::string const & key, std::string const & val )
      {
        entries [key] = val;
      }
      void put ( key_desc_t const & key, std::string const & val )
      {
        put( detail::flatten( key ), val );
      }

      void del ( std::string const & key )
      {
        entries.erase( key );
      }
      void del ( key_desc_t const & key )
      {
        del( detail::flatten( key ) );
      }

      void write (std::ostream & o)
      {
        std::string cursec;
        std::string curid;
        for ( typename entries_t::const_iterator kv (entries.begin())
            ; kv != entries.end()
            ; ++kv
            )
        {
          key_desc_t k (detail::unflatten (kv->first));
          if (k.sec != cursec || (k.id && (*k.id != curid)))
          {
            if (!cursec.empty())
            {
              o << std::endl;
            }
            // new section
            o << "[";
            o << k.sec;
            if (k.id)
            {
              o << " \"" << *k.id << "\"";
              curid = *k.id;
            }
            o << "]";
            o << std::endl;
            cursec = k.sec;
          }

          o << "  ";
          o << k.key;
          o << " = ";
          o << kv->second;
          o << std::endl;
        }
      }

      entries_t entries;
    };
  }
}
}
}

#endif
