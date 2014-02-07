#ifndef GPI_SPACE_PC_CONTAINER_MANAGER_HPP
#define GPI_SPACE_PC_CONTAINER_MANAGER_HPP 1

#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#include <gpi-space/pc/type/counter.hpp>
#include <gpi-space/pc/container/process.hpp>
#include <gpi-space/pc/container/connector.hpp>
#include <gpi-space/pc/memory/manager.hpp>
#include <gpi-space/pc/global/topology.hpp>

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      class process_t;

      class manager_t
      {
      public:
        explicit
        manager_t ( std::string const & path_to_socket
                  , std::vector<std::string> const& default_memory_urls
                  );

        ~manager_t ();

        // api
        void handle_new_connection (int fd);
        void handle_process_error ( const gpi::pc::type::process_id_t proc_id
                                  , int error
                                  );
      private:
        typedef boost::shared_ptr<process_t> process_ptr_t;
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        void detach_process (const gpi::pc::type::process_id_t);
        void detach_memory_from_process (const gpi::pc::type::process_id_t);

        connector_t m_connector;
        gpi::pc::type::counter_t m_process_counter;

        mutable mutex_type _mutex_processes;
        boost::unordered_map<gpi::pc::type::process_id_t, process_ptr_t>
          m_processes;
      };
    }
  }
}

#endif
