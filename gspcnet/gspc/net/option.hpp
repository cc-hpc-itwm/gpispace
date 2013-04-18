#ifndef GSPC_NET_OPTION_HPP
#define GSPC_NET_OPTION_HPP

#include <string>
#include <fhg/util/url.hpp>
#include <boost/system/error_code.hpp>

namespace gspc
{
  namespace net
  {
    typedef fhg::util::url_t::arg_map_t option_map_t;

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
  }
}

#endif
