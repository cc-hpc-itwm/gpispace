#ifndef GSPC_NET_TESTS_MOCK_USER_HPP
#define GSPC_NET_TESTS_MOCK_USER_HPP

#include <gspc/net/frame.hpp>
#include <gspc/net/server/user.hpp>

#include <list>

namespace gspc
{
  namespace net
  {
    namespace tests
    {
      namespace mock
      {
        class user : public gspc::net::server::user_t
        {
        public:
          typedef std::list<frame> frame_list_t;

          virtual ~user () {}

          virtual int deliver (frame const & f)
          {
            frames.push_back (f);
            return 0;
          }

          frame_list_t frames;
        };
      }
    }
  }
}

#endif
