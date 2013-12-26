#ifndef FHG_UTIL_THREAD_ATOMIC
#define FHG_UTIL_THREAD_ATOMIC

#ifdef HAS_STD_ATOMIC
#  include <atomic>
#else
#  include <boost/thread/mutex.hpp>
#  include <boost/thread/locks.hpp>
#  include <boost/utility.hpp>
#endif

namespace fhg
{
  namespace thread
  {
#ifdef HAS_STD_ATOMIC
    using std::atomic;
#else
    template <typename value_type>
    class atomic : boost::noncopyable
    {
      typedef boost::lock_guard<boost::mutex> lock_type;

    public:
      atomic()
        : _value()
      {}

      explicit
      atomic (value_type v)
        : _value (v)
      {}

      operator value_type() const
      {
        lock_type const _ (_mutex);
        return _value;
      }

      value_type operator++ ()
      {
        lock_type const _ (_mutex);
        return ++_value;
      }

    private:
      mutable boost::mutex _mutex;
      value_type _value;
    };
#endif
  }
}

#endif
