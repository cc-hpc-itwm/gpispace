#ifndef GPI_SPACE_PC_CONTAINER_PROCESS_HPP
#define GPI_SPACE_PC_CONTAINER_PROCESS_HPP 1

#include <boost/thread.hpp>
#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/memory_location.hpp>

#include <gpi-space/pc/proto/message.hpp>

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      class manager_t;

      class process_t : boost::noncopyable
      {
      public:
        explicit
        process_t ( manager_t & mgr
                  , const gpi::pc::type::process_id_t id
                  , const int socket
                  )
          : m_mgr (mgr)
          , m_id (id)
          , m_socket (socket)
          , m_reader
            (boost::bind (&process_t::reader_thread_main, this, m_socket))
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

        gpi::pc::type::process_id_t const& id() const
        {
          return m_id;
        }

        // protocol implementation

        void                        del_memory (gpi::pc::type::segment_id_t);

        void collect_info (gpi::pc::type::info::descriptor_t &);
      private:
        void reader_thread_main (const int fd);

        int receive ( const int fd
                     , gpi::pc::proto::message_t & msg
                     , const size_t max_size = (1 << 20)
                     );
        int send (const int fd, gpi::pc::proto::message_t const & msg);

        int close_socket (const int fd);
        int checked_read (const int fd, void *buf, const size_t len);

        manager_t & m_mgr;
        const gpi::pc::type::process_id_t m_id;
        int const m_socket;
        boost::thread m_reader;
      };
    }
  }
}

#endif
