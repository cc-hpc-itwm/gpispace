#ifndef GPI_SPACE_PC_CONTAINER_PROCESS_HPP
#define GPI_SPACE_PC_CONTAINER_PROCESS_HPP 1

#include <boost/thread.hpp>
#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/memory_location.hpp>

#include <gpi-space/pc/proto/message.hpp>
#include <gpi-space/pc/memory/manager.hpp>

#include <boost/function.hpp>

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      class process_t : boost::noncopyable
      {
      public:
        process_t
          ( boost::function<void (gpi::pc::type::process_id_t const&, int)>
            const& handle_process_error
          , const gpi::pc::type::process_id_t id
          , const int socket
          , memory::manager_t& memory_manager
          )
          : m_handle_process_error (handle_process_error)
          , m_id (id)
          , m_socket (socket)
          , m_reader
            (boost::bind (&process_t::reader_thread_main, this, m_socket))
          , _memory_manager (memory_manager)
        {}
        ~process_t()
        {
          close_socket (m_socket);

          if (boost::this_thread::get_id() != m_reader.get_id())
          {
            if (m_reader.joinable())
            {
              m_reader.join ();
            }
          }
        }

      private:
        void reader_thread_main (const int fd);

        int receive ( const int fd
                     , gpi::pc::proto::message_t & msg
                     , const size_t max_size = (1 << 20)
                     );
        int send (const int fd, gpi::pc::proto::message_t const & msg);

        int close_socket (const int fd);
        int checked_read (const int fd, void *buf, const size_t len);

        boost::function <void (gpi::pc::type::process_id_t const&, int)> const&
          m_handle_process_error;
        const gpi::pc::type::process_id_t m_id;
        int const m_socket;
        boost::thread m_reader;
        memory::manager_t& _memory_manager;
      };
    }
  }
}

#endif
