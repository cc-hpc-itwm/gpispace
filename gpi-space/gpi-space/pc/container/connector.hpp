#ifndef GPI_SPACE_PC_CONTAINER_CONNECTOR_HPP
#define GPI_SPACE_PC_CONTAINER_CONNECTOR_HPP 1

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>

#include <gpi-space/pc/type/typedefs.hpp>

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      template <typename Manager>
      class connector_t : boost::noncopyable
      {
      public:
        typedef Manager manager_type;
        typedef connector_t<manager_type> self;

        explicit
        connector_t (manager_type & mgr, std::string const & p)
          : m_mgr (mgr)
          , m_path (p)
          , m_socket (-1)
          , m_stopping (false)
        {}

        ~connector_t ();

        void start ();
        void stop ();
      private:
        typedef boost::shared_ptr<boost::thread> thread_t;
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        void listener_thread_main (const int fd);
        void start_thread ();
        void stop_thread ();

        int close_socket (const int fd);
        int open_socket(std::string const & path);
        int safe_unlink(std::string const & path);

        void handle_new_connection (int fd);

        mutex_type m_mutex;
        manager_type & m_mgr;
        std::string m_path;
        thread_t m_listener;
        int m_socket;
        bool m_stopping;
      };
    }
  }
}

#ifdef GPI_SPACE_HEADER_ONLY
#  include "connector.ipp"
#endif

#endif
