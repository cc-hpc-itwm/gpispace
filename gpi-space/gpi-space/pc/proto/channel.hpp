#ifndef GPI_SPACE_PC_PROTO_CHANNEL_HPP
#define GPI_SPACE_PC_PROTO_CHANNEL_HPP

#include <deque>
#include <map>

#include <boost/thread.hpp>

#include <fhg/util/thread/semaphore.hpp>
#include <gpi-space/pc/proto/message.hpp>

namespace gpi
{
  namespace socket
  {
    class channel_t
    {
    public:
      typedef boost::posix_time::time_duration time_duration;
      typedef gpi::pc::proto::header_t header_t;
      typedef gpi::pc::proto::complete_message_t message_t;

      explicit
      channel_t (const int fd, const std::size_t backlog_size = 1024);

      int request(const message_t & req, message_t & rpl);
      int request( const message_t & req, message_t & rpl
                 , const time_duration timeout
                 );

      int recv (message_t & msg);
      int recv (message_t & msg, const time_duration timeout);

      int reply(const header_t & hdr, message_t msg);
    private:
      typedef boost::shared_ptr<boost::thread> thread_ptr;
      typedef boost::mutex mutex_type;
      typedef boost::unique_lock<mutex_type> ulock_type;
      typedef boost::shared_lock<mutex_type> slock_type;
      typedef boost::condition_variable condition_type;

      struct wait_on_reply
      {
        uint32_t message_id;
        message_t & buffer;

        void wait(time_duration timeout);
        void notify();
      };

      int m_socket;
      std::size_t m_backlog;

      thread_ptr m_reader;
      thread_ptr m_writer;
    };
  }
}

#endif
