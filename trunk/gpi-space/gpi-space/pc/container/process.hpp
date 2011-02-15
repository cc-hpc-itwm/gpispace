#ifndef GPI_SPACE_PC_CONTAINER_PROCESS_HPP
#define GPI_SPACE_PC_CONTAINER_PROCESS_HPP 1

#include <boost/thread.hpp>
#include <gpi-space/pc/type/typedefs.hpp>

#include <gpi-space/pc/proto/message.hpp>

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      template <typename Manager>
      class process_t : boost::noncopyable
      {
      public:
        typedef Manager manager_type;
        typedef process_t<manager_type> self;

        explicit
        process_t ( manager_type & mgr
                  , const gpi::pc::type::process_id_t id
                  , const int socket
                  )
          : m_mgr (mgr)
          , m_id (id)
          , m_socket (socket)
        {}

        gpi::pc::type::process_id_t get_id () const;
        void start ();
        void stop ();
      private:
        typedef boost::shared_ptr<boost::thread> thread_t;
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        void reader_thread_main (const int fd);
        void start_thread ();
        void stop_thread ();

        int receive ( const int fd
                     , gpi::pc::proto::message_t & msg
                     , const size_t max_size = (1 << 20)
                     );
        int send (const int fd, gpi::pc::proto::message_t const & msg);

        gpi::pc::proto::message_t handle_message (const gpi::pc::proto::message_t &);

        int close_socket (const int fd);
        int checked_read (const int fd, void *buf, const size_t len);
        void decode_buffer (const char *buf, const size_t len, gpi::pc::proto::message_t &);

        mutex_type m_mutex;
        manager_type & m_mgr;
        const gpi::pc::type::process_id_t m_id;
        int m_socket;
        thread_t m_reader;
      };
    }
  }
}

#include "process.ipp"

#endif
