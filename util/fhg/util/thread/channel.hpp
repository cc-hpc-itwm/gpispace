#ifndef FHG_UTIL_THREAD_CHANNEL_HPP
#define FHG_UTIL_THREAD_CHANNEL_HPP

#include <deque>
#include <fhg/util/thread/queue.hpp>

namespace fhg
{
  namespace thread
  {
    template <typename T>
    class channel : public fhg::thread::queue<T, std::list>
    {
    public:
      typedef queue<T, std::list>           super_type;
      typedef typename super_type::size_type size_type;

      channel ()
        : super_type ()
      {}

      channel (size_type buffer_size)
        : super_type (buffer_size)
      {}
    };

    template <typename T, typename U>
    channel<T> & operator << (channel<T> & chan, U const & val)
    {
      chan.put (val);
      return chan;
    }

    template <typename T, typename U>
    channel<T> & operator >> (channel<T> & chan, U & val)
    {
      val = chan.get ();
      return chan;
    }

    template <typename T, typename U>
    channel<T> & operator << (channel<T> & lhs, channel<U> & rhs)
    {
      lhs.put (rhs.get ());
      return lhs;
    }

    template <typename T, typename U>
    channel<U> & operator >> (channel<T> & lhs, channel<U> & rhs)
    {
      rhs.put (lhs.get ());
      return rhs;
    }
  }
}

#endif
