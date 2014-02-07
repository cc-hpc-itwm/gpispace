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
        typedef manager_t self;

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

        gpi::pc::type::handle_t
        alloc ( const gpi::pc::type::process_id_t proc_id
              , const gpi::pc::type::segment_id_t
              , const gpi::pc::type::size_t
              , const std::string & name
              , const gpi::pc::type::flags_t
              );

        void free ( const gpi::pc::type::process_id_t
                  , const gpi::pc::type::handle_id_t
                  );
        gpi::pc::type::handle::descriptor_t info ( const gpi::pc::type::process_id_t
                                                 , const gpi::pc::type::handle_id_t
                                                 ) const;

        void list_allocations ( const gpi::pc::type::process_id_t proc_id
                              , const gpi::pc::type::segment_id_t
                              , gpi::pc::type::handle::list_t &
                              ) const;

        gpi::pc::type::queue_id_t
        memcpy ( const gpi::pc::type::process_id_t proc_id
               , gpi::pc::type::memory_location_t const & dst
               , gpi::pc::type::memory_location_t const & src
               , const gpi::pc::type::size_t amount
               , const gpi::pc::type::queue_id_t queue
               );

        gpi::pc::type::segment_id_t
        register_segment ( const gpi::pc::type::process_id_t proc_id
                         , std::string const & name
                         , const gpi::pc::type::size_t sz
                         , const gpi::pc::type::flags_t flags
                         );
        void unregister_segment ( const gpi::pc::type::process_id_t proc_id
                                , const gpi::pc::type::segment_id_t
                                );
        void attach_process_to_segment ( const gpi::pc::type::process_id_t proc_id
                                       , const gpi::pc::type::segment_id_t id
                                       );
        void detach_process_from_segment ( const gpi::pc::type::process_id_t proc_id
                                         , const gpi::pc::type::segment_id_t id
                                         );
        void list_segments ( const gpi::pc::type::process_id_t
                           , gpi::pc::type::segment::list_t &
                           ) const;
        void collect_info (gpi::pc::type::info::descriptor_t &) const;

        gpi::pc::type::size_t
        wait_on_queue ( const gpi::pc::type::process_id_t proc_id
                      , const gpi::pc::type::queue_id_t queue
                      );

        gpi::pc::type::segment_id_t
        add_memory ( const gpi::pc::type::process_id_t proc_id
                   , std::string const & url
                   );
        void
        del_memory ( const gpi::pc::type::process_id_t proc_id
                   , const gpi::pc::type::segment_id_t
                   );
      private:
        typedef boost::shared_ptr<process_t> process_ptr_t;
        typedef boost::unordered_map< gpi::pc::type::process_id_t
                                    , process_ptr_t
                                    > process_map_t;
        typedef std::list<process_ptr_t> process_list_t;
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        void detach_process (const gpi::pc::type::process_id_t);
        void detach_memory_from_process (const gpi::pc::type::process_id_t);
        void initialize_memory_manager ();

        /*                                                    */
        /*           M E M B E R    V A R I A B L E S         */
        /*                                                    */

        mutable mutex_type m_mutex;
        connector_t m_connector;
        gpi::pc::type::counter_t m_process_counter;
        std::vector<std::string> m_default_memory_urls;

        process_map_t m_processes;
      };
    }
  }
}

#endif
