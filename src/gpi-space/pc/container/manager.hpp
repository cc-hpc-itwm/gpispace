#ifndef GPI_SPACE_PC_CONTAINER_MANAGER_HPP
#define GPI_SPACE_PC_CONTAINER_MANAGER_HPP 1

#include <boost/thread.hpp>

#include <gpi-space/pc/type/counter.hpp>
#include <gpi-space/pc/container/process.hpp>
#include <gpi-space/pc/memory/manager.hpp>
#include <gpi-space/pc/global/topology.hpp>

#include <map>
#include <memory>
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
      class manager_t : boost::noncopyable
      {
      public:
        manager_t ( std::string const & p
                    , std::vector<std::string> const& default_memory_urls
                  , api::gpi_api_t& gpi_api
                  , std::unique_ptr<fhg::com::peer_t> topology_peer
                    );

        ~manager_t ();

      private:
        typedef boost::shared_ptr<boost::thread> thread_t;

        void listener_thread_main();

        void close_socket (const int fd);
        int safe_unlink(std::string const & path);

        std::string m_path;
        thread_t m_listener;
        int m_socket;
        bool m_stopping;

        gpi::pc::type::counter_t m_process_counter;
        mutable boost::mutex _mutex_processes;
        std::map<gpi::pc::type::process_id_t, std::unique_ptr<process_t>>
          m_processes;

        void handle_process_error ( const gpi::pc::type::process_id_t proc_id
                                  , int error
                                  );
        void detach_process ( const gpi::pc::type::process_id_t
                            , bool called_from_dtor = false
                            );

        gpi::api::gpi_api_t& _gpi_api;
        memory::manager_t _memory_manager;
        global::topology_t _topology;
      };
    }
  }
}

#endif
