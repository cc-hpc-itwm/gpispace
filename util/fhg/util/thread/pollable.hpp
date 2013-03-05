#ifndef FHG_UTIL_THREAD_POLLABLE_HPP
#define FHG_UTIL_THREAD_POLLABLE_HPP

namespace fhg
{
  namespace thread
  {
    enum poll_mode_t
      {
        POLLIN   = 0x01
      , POLLOUT  = 0x02
      };

    class pollable
    {
    public:
      virtual ~pollable () {}

      virtual int poll () const = 0;
    };
  }
}

#endif
