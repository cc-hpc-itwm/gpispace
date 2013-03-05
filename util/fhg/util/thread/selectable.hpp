#ifndef FHG_UTIL_THREAD_SELECTABLE_HPP
#define FHG_UTIL_THREAD_SELECTABLE_HPP

namespace fhg
{
  namespace thread
  {
    enum select_mode_t
      {
        POLL_IN   = 0x01
      , POLL_OUT  = 0x02
      };

    class selectable
    {
    public:
      virtual ~selectable () {}

      virtual bool is_ready_for (select_mode_t) const = 0;
    };
  }
}

#endif
