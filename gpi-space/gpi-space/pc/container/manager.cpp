#include "manager.hpp"

#include <fhglog/LogMacros.hpp>

#include <gpi-space/gpi/api.hpp>
#include <gpi-space/pc/container/process.hpp>
#include <gpi-space/pc/global/topology.hpp>
#include <gpi-space/pc/segment/segment.hpp>
#include <gpi-space/pc/memory/manager.hpp>

#include <boost/foreach.hpp>

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      manager_t::manager_t ( std::string const & p
                           , std::vector<std::string> const& default_memory_urls
                           )
       : m_connector (*this, p)
       , m_process_counter (0)
      {
        if ( default_memory_urls.size ()
           >= gpi::pc::memory::manager_t::MAX_PREALLOCATED_SEGMENT_ID
           )
        {
          throw std::runtime_error ("too many predefined memory urls!");
        }

        gpi::api::gpi_api_t & gpi_api (gpi::api::gpi_api_t::get());
        global::topology().start( gpi_api.rank()
                                , global::topology_t::any_addr()
                                , global::topology_t::any_port() // topology_t::port_t("10821")
                                , "dummy-cookie"
                                );

        for (std::size_t n(0); n < gpi_api.number_of_nodes(); ++n)
        {
          if (gpi_api.rank() != n)
            global::topology().add_child(n);
        }

        if (gpi_api.is_master ())
        {
          global::topology().establish();
        }
        global::memory_manager ().start ( gpi_api.rank ()
                                        , gpi_api.number_of_queues ()
                                        );

        if (global::topology ().is_master ())
        {
          gpi::pc::type::id_t id = 1;
          BOOST_FOREACH (std::string const& url, default_memory_urls)
          {
            global::memory_manager ().add_memory
              ( 0 // owner
              , url
              , id
              );
            ++id;
          }
        }

          if (global::topology().is_master ())
          {
            global::topology ().go ();
          }
          else
          {
            global::topology ().wait_for_go ();
          }

          m_connector.start ();
      }

      manager_t::~manager_t ()
      {
          m_connector.stop ();
          while (! m_processes.empty())
          {
            detach_process (m_processes.begin()->first);
          }

          global::memory_manager().clear();
        global::topology().stop();
      }

      void manager_t::detach_process (const gpi::pc::type::process_id_t id)
      {
        boost::mutex::scoped_lock const _ (_mutex_processes);

        if (m_processes.find (id) == m_processes.end())
        {
          CLOG( ERROR
              , "gpi.container"
              , "process id already detached!"
              );
          throw std::runtime_error ("no such process");
        }

        m_processes.erase (id);

        global::memory_manager().garbage_collect (id);

        CLOG( INFO
            , "gpi.container"
            , "process container " << id << " detached"
            );
      }

      void manager_t::handle_new_connection (int fd)
      {
        //! \note must be lvalue to be used in ptr_map::insert
        gpi::pc::type::process_id_t id (m_process_counter.inc());

        {
          boost::mutex::scoped_lock const _ (_mutex_processes);

          m_processes.insert (id, new process_t (*this, id, fd));
        }

        CLOG( INFO
            , "gpi.container"
            , "process container " << id << " attached"
            );
      }

      void manager_t::handle_process_error( const gpi::pc::type::process_id_t proc_id
                                          , int error
                                          )
      {
          LOG_IF ( ERROR
                 , error != 0
                 , "process container " << proc_id << " died: " << strerror (error)
                 );
          detach_process (proc_id);
      }
    }
  }
}
