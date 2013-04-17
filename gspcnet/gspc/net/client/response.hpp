#ifndef GSPC_NET_CLIENT_RESPONSE_HPP
#define GSPC_NET_CLIENT_RESPONSE_HPP

#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/system/error_code.hpp>
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
        explicit
        response_t (std::string const &id);

        void wait ();
        bool wait (boost::posix_time::time_duration timeout);

        void notify (frame const &);
        void abort (boost::system::error_code const &ec);
        boost::system::error_code const & error () const;
        std::string const &id () const;

        boost::optional<frame> const & get_reply () const;
        bool is_aborted () const;
      private:
        bool                      m_aborted;
        boost::optional<frame>    m_reply;
        mutable boost::mutex      m_mutex;
        boost::condition_variable m_wait_object;
        std::string               m_id;
        boost::system::error_code m_ec;
      };
    }
  }
}

#endif
