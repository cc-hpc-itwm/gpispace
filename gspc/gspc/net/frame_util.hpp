#ifndef GSPC_NET_FRAME_UTIL_HPP
#define GSPC_NET_FRAME_UTIL_HPP

#include <boost/lexical_cast.hpp>

#include <gspc/net/frame.hpp>

namespace gspc
{
  namespace net
  {
    template <typename T>
    frame & frame_set_header ( frame & f
                             , frame::key_type const &key
                             , T const &val
                             )
    {
      return f.set_header (key, boost::lexical_cast<frame::value_type>(val));
    }

    template <typename T>
    T frame_get_header ( frame const & f
                       , frame::key_type const &key
                       , T const &dflt
                       )
    {
      boost::optional<std::string> entry = f.get_header (key);
      if (entry)
      {
        return boost::lexical_cast<T>(*entry);
      }
      else
      {
        return dflt;
      }
    }

    bool is_heartbeat (frame const &f);
  }
}

#endif
