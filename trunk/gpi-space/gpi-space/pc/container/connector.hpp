#ifndef GPI_SPACE_PC_CONTAINER_CONNECTOR_HPP
#define GPI_SPACE_PC_CONTAINER_CONNECTOR_HPP 1

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/container/process.hpp>

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      template <typename Manager, typename Process>
      class connector_t : boost::noncopyable
      {
      public:
        typedef Manager manager_type;
        typedef Process process_type;
        typedef connector_t<manager_type, process_type> self;

        explicit
        connector_t (manager_type & mgr, std::string const & p)
          : m_mgr (mgr)
          , m_path (p)
          , m_socket (-1)
        {}

        void start ();
        void stop ();
      private:
        typedef boost::shared_ptr<boost::thread> thread_t;

        void listener_thread ();

        manager_type & m_mgr;
        std::string m_path;
        thread_t m_listener;
        int m_socket;
      };
    }
  }
}

#include "connector.ipp"

#endif
