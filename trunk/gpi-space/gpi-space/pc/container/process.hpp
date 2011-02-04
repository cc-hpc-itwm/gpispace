#ifndef GPI_SPACE_PC_CONTAINER_PROCESS_HPP
#define GPI_SPACE_PC_CONTAINER_PROCESS_HPP 1

#include <boost/thread.hpp>
#include <gpi-space/pc/type/typedefs.hpp>

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
        explicit
        process_t ( manager_type & mgr
                  , const gpi::pc::type::id_t id
                  , const int socket
                  )
          : m_mgr (mgr)
          , m_id (id)
          , m_socket (socket)
        {}

        void start ();
        void stop ();
      private:
        void reader_thread ();

        manager_type & m_mgr;
        gpi::pc::type::id_t m_id;
        int m_socket;
        boost::thread m_reader;
      };
    }
  }
}

#include "process.ipp"

#endif
