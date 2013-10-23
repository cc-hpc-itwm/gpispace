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

    struct set_header
    {
      explicit
      set_header (std::string const &k, std::string const &v)
        : key (k)
        , value (v)
      {}

      const std::string key;
      const std::string value;
    };

    class stream
    {
    public:
      explicit stream (frame & f)
        : m_frame (f)
        , m_sstr ()
      {}

      ~stream ()
      {
        m_frame.add_body (m_sstr.str ());
      }

      frame & get_frame () { return m_frame; }

      template <class T>
      stream &operator << (T const &t)
      {
        m_sstr << t;
        return *this;
      }

      stream & operator << (set_header const &hdr)
      {
        m_frame.set_header (hdr.key, hdr.value);
        return *this;
      }

      typedef std::ostream & (*ostream_manip)(std::ostream &);
      stream &operator << (ostream_manip manip)
      {
        manip (m_sstr);
        return *this;
      }

      typedef stream & (*stream_manip)(stream &);
      stream &operator << (const stream_manip &manip)
      {
        return manip (*this);
      }
    private:
      frame & m_frame;
      std::ostringstream m_sstr;
    };
  }
}

#endif
