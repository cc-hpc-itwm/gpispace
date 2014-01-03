#ifndef GSPC_NET_FRAME_UTIL_HPP
#define GSPC_NET_FRAME_UTIL_HPP

#include <boost/lexical_cast.hpp>

#include <gspc/net/frame.hpp>

namespace gspc
{
  namespace net
  {
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
