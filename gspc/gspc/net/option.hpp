#ifndef GSPC_NET_OPTION_HPP
#define GSPC_NET_OPTION_HPP

#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include <map>

namespace gspc
{
  namespace net
  {
    typedef std::map<std::string, std::string> option_map_t;

    template <typename T>
    T get_option ( option_map_t const &opts
                 , std::string const & key
                 , T const &dflt
                 , boost::system::error_code & ec
                 )
    {
      using namespace boost::system;

      option_map_t::const_iterator it = opts.find (key);
      if (it != opts.end ())
      {
        try
        {
          return boost::lexical_cast<T>(it->second);
        }
        catch (boost::bad_lexical_cast const &)
        {
          ec = errc::make_error_code (errc::invalid_argument);
          return dflt;
        }
      }
      else
      {
        return dflt;
      }
    }

    template <typename T>
    T get_option ( option_map_t const &opts
                 , std::string const & key
                 , T const &dflt
                 )
    {
      using namespace boost::system;

      option_map_t::const_iterator it = opts.find (key);
      if (it != opts.end ())
      {
        try
        {
          return boost::lexical_cast<T>(it->second);
        }
        catch (boost::bad_lexical_cast const &)
        {
          throw boost::system::system_error
            (errc::make_error_code (errc::invalid_argument));
        }
      }
      else
      {
        return dflt;
      }
    }
  }
}

#endif
