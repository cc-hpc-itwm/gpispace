#include "manager.hpp"

#include <boost/bind.hpp>

#include <fhglog/LogMacros.hpp>

#include <gpi-space/gpi/api.hpp>
#include <gpi-space/pc/container/process.hpp>
#include <gpi-space/pc/global/topology.hpp>
#include <gpi-space/pc/segment/segment.hpp>
#include <gpi-space/pc/memory/manager.hpp>

#include <gpi-space/pc/memory/factory.hpp>

// TODO remove this - currently required for register_memory
#include <fhg/util/url.hpp>
#include <fhg/util/url_io.hpp>

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
       , m_default_memory_urls (default_memory_urls)
      {
        if ( m_default_memory_urls.size ()
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
          typedef std::vector<std::string> url_list_t;
          url_list_t::iterator it = m_default_memory_urls.begin ();
          url_list_t::iterator end = m_default_memory_urls.end ();
          gpi::pc::type::id_t id = 1;
          for ( ; it != end ; ++it)
          {
            global::memory_manager ().add_memory
              ( 0 // owner
              , *it
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
        lock_type lock (m_mutex);

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
        gpi::pc::type::counter_t const id (m_process_counter.inc());

        {
          lock_type lock (m_mutex);

          m_processes[id] = process_ptr_t (new process_t (*this, id, fd));
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

      gpi::pc::type::segment_id_t manager_t::register_segment ( const gpi::pc::type::process_id_t pc_id
                                                              , std::string const & name
                                                              , const gpi::pc::type::size_t sz
                                                              , const gpi::pc::type::flags_t flags
                                                              )
      {
        using namespace gpi::pc;

        fhg::util::url_t url;
        url.type ("shm");
        url.path (name);
        url.set ("size", boost::lexical_cast<std::string>(sz));
        if (flags & F_PERSISTENT)
          url.set ("persistent", "true");
        if (flags & F_EXCLUSIVE)
          url.set ("exclusive", "true");

        memory::area_ptr_t area =
          memory::factory ().create (boost::lexical_cast<std::string>(url));
        area->set_owner (pc_id);
        return global::memory_manager().register_memory (pc_id, area);
      }

      void manager_t::unregister_segment ( const gpi::pc::type::process_id_t proc_id
                                         , const gpi::pc::type::segment_id_t seg_id
                                         )
      {
        global::memory_manager().unregister_memory (proc_id, seg_id);
      }

      void manager_t::list_segments ( const gpi::pc::type::process_id_t
                                    , gpi::pc::type::segment::list_t &l
                                    ) const
      {
        global::memory_manager().list_memory (l);
      }

      void manager_t::attach_process_to_segment ( const gpi::pc::type::process_id_t proc_id
                                                , const gpi::pc::type::segment_id_t seg_id
                                                )
      {
        global::memory_manager().attach_process (proc_id, seg_id);
      }

      void manager_t::detach_process_from_segment ( const gpi::pc::type::process_id_t proc_id
                                                  , const gpi::pc::type::segment_id_t seg_id
                                                  )
      {
        global::memory_manager().detach_process (proc_id, seg_id);
      }

      void
      manager_t::collect_info (gpi::pc::type::info::descriptor_t &info) const
      {
        gpi::api::gpi_api_t & gpi_api (gpi::api::gpi_api_t::get());

        info.rank = gpi_api.rank();
        info.nodes = gpi_api.number_of_nodes();
        info.queues = gpi_api.number_of_queues();
        info.queue_depth = gpi_api.queue_depth();
      }

      gpi::pc::type::handle_t
      manager_t::alloc ( const gpi::pc::type::process_id_t proc_id
                       , const gpi::pc::type::segment_id_t seg_id
                       , const gpi::pc::type::size_t size
                       , const std::string & name
                       , const gpi::pc::type::flags_t flags
                       )
      {
        return global::memory_manager().alloc ( proc_id
                                              , seg_id
                                              , size
                                              , name
                                              , flags
                                              );
      }

      void
      manager_t::free ( const gpi::pc::type::process_id_t
                      , const gpi::pc::type::handle_id_t hdl
                      )
      {
        global::memory_manager().free (hdl);
      }

      gpi::pc::type::handle::descriptor_t
      manager_t::info ( const gpi::pc::type::process_id_t
                      , const gpi::pc::type::handle_id_t hdl
                      ) const
      {
        return global::memory_manager().info (hdl);
      }

      void
      manager_t::list_allocations ( const gpi::pc::type::process_id_t proc_id
                                  , const gpi::pc::type::segment_id_t seg_id
                                  , gpi::pc::type::handle::list_t & list
                                  ) const
      {
        if (seg_id == gpi::pc::type::segment::SEG_INVAL)
          global::memory_manager().list_allocations(proc_id, list);
        else
          global::memory_manager().list_allocations (proc_id, seg_id, list);
      }

      gpi::pc::type::queue_id_t
      manager_t::memcpy ( const gpi::pc::type::process_id_t proc_id
                        , gpi::pc::type::memory_location_t const & dst
                        , gpi::pc::type::memory_location_t const & src
                        , const gpi::pc::type::size_t amount
                        , const gpi::pc::type::queue_id_t queue
                        )
      {
        gpi::pc::type::validate (dst.handle);
        gpi::pc::type::validate (src.handle);
        return global::memory_manager().memcpy (proc_id, dst, src, amount, queue);
      }

      gpi::pc::type::size_t
      manager_t::wait_on_queue ( const gpi::pc::type::process_id_t proc_id
                               , const gpi::pc::type::queue_id_t queue
                               )
      {
        return global::memory_manager().wait_on_queue (proc_id, queue);
      }

      gpi::pc::type::segment_id_t
      manager_t::add_memory ( const gpi::pc::type::process_id_t proc_id
                            , std::string const &url
                            )
      {
        return global::memory_manager ().add_memory (proc_id, url);
      }

      void
      manager_t::del_memory ( const gpi::pc::type::process_id_t proc_id
                            , gpi::pc::type::segment_id_t seg_id
                            )
      {
        global::memory_manager ().del_memory (proc_id, seg_id);
      }
    }
  }
}
