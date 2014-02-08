#ifndef GPI_SPACE_PC_CONTAINER_MANAGER_HPP
#define GPI_SPACE_PC_CONTAINER_MANAGER_HPP 1

#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/ptr_container/ptr_map.hpp>

#include <gpi-space/pc/type/counter.hpp>
#include <gpi-space/pc/container/process.hpp>
#include <gpi-space/pc/memory/manager.hpp>
#include <gpi-space/pc/global/topology.hpp>

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
      class connector_t : boost::noncopyable
      {
      public:
        explicit
        connector_t ( boost::function<void (int)> const& handle_new_connection
                    , std::string const & p
                    )
          : m_handle_new_connection (handle_new_connection)
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

        int close_socket (const int fd);
        int open_socket(std::string const & path);
        int safe_unlink(std::string const & path);

        mutex_type m_mutex;
        boost::function<void (int)> const& m_handle_new_connection;
        std::string m_path;
        thread_t m_listener;
        int m_socket;
        bool m_stopping;
      };

      class process_t;

      class manager_t
      {
      public:
        manager_t ( std::string const & path_to_socket
                  , std::vector<std::string> const& default_memory_urls
                  );

        ~manager_t ();

      private:
        void handle_new_connection (int fd);
        void handle_process_error ( const gpi::pc::type::process_id_t proc_id
                                  , int error
                                  );
        void detach_process (const gpi::pc::type::process_id_t);

        connector_t m_connector;
        gpi::pc::type::counter_t m_process_counter;

        mutable boost::mutex _mutex_processes;
        boost::ptr_map<gpi::pc::type::process_id_t, process_t>
          m_processes;
      };
    }
  }
}

#endif
