#ifndef GPI_SPACE_PC_CONTAINER_MANAGER_HPP
#define GPI_SPACE_PC_CONTAINER_MANAGER_HPP 1

#include <boost/thread.hpp>
#include <boost/ptr_container/ptr_map.hpp>

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
        manager_t ( std::string const & path_to_socket
                  , std::vector<std::string> const& default_memory_urls
                  );

        ~manager_t ();

        void handle_new_connection (int fd);
      private:
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
