#ifndef GSPC_NET_CLIENT_RESPONSE_HPP
#define GSPC_NET_CLIENT_RESPONSE_HPP

#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <gspc/net/frame.hpp>

namespace gspc
{
  namespace net
  {
    namespace client
    {
      class response_t : boost::noncopyable
      {
      public:
        response_t ();

        void wait ();
        bool wait (boost::posix_time::time_duration timeout);

        void notify (frame const &);
        void abort ();

        boost::optional<frame> const & get_reply () const;
        bool is_aborted () const;
      private:
        bool                      m_aborted;
        boost::optional<frame>    m_reply;
        mutable boost::mutex      m_mutex;
        boost::condition_variable m_wait_object;
      };
    }
  }
}

#endif
