#ifndef GSPC_NET_TESTS_MOCK_USER_HPP
#define GSPC_NET_TESTS_MOCK_USER_HPP

#include <gspc/net/frame.hpp>
#include <gspc/net/user.hpp>

#include <list>

#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

namespace gspc
{
  namespace net
  {
    namespace tests
    {
      namespace mock
      {
        class user : public gspc::net::user_t
        {
        public:
          user ()
          {
            static size_t id_counter = 0;
            m_id = id_counter++;
          }

          typedef std::list<frame> frame_list_t;

          virtual ~user () {}

          virtual int deliver (frame const & f)
          {
            boost::lock_guard<boost::mutex> lock (m_mutex);
            frames.push_back (f);
            return 0;
          }

          virtual size_t id () const
          {
            return m_id;
          }

          frame_list_t frames;

        private:
          boost::mutex m_mutex;
          size_t m_id;
        };
      }
    }
  }
}

#endif
