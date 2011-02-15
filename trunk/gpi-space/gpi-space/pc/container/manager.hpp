#ifndef GPI_SPACE_PC_CONTAINER_MANAGER_HPP
#define GPI_SPACE_PC_CONTAINER_MANAGER_HPP 1

#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#include <gpi-space/pc/container/process.hpp>
#include <gpi-space/pc/container/connector.hpp>
#include <gpi-space/pc/segment/manager.hpp>

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      class manager_t
      {
        enum state_t
          {
            ST_STOPPED,
            ST_STARTING,
            ST_STARTED,
            ST_STOPPING,
            NUM_STATES,
          };

      public:
        typedef manager_t self;
        typedef gpi::pc::segment::manager_t segment_manager_type;
        typedef gpi::pc::container::process_t<manager_t> process_type;
        typedef gpi::pc::container::connector_t<manager_t> connector_type;

        explicit
        manager_t (std::string const & p)
          : m_state (ST_STOPPED)
          , m_connector (*this, p)
          , m_process_id (0)
        {}

        ~manager_t ();

        void start();
        void stop ();

        // api
        void handle_new_connection (int fd);
        void handle_process_error ( const gpi::pc::type::process_id_t proc_id
                                  , int error
                                  );
        void handle_connector_error (int error);

        gpi::pc::type::segment_id_t register_segment ( const gpi::pc::type::process_id_t proc_id
                                                     , std::string const & name
                                                     , const gpi::pc::type::size_t sz
                                                     , const gpi::pc::type::flags_t flags
                                                     );

        gpi::pc::type::segment::list_t list_segments () const;

      private:
        typedef boost::shared_ptr<process_type> process_ptr_t;
        typedef boost::unordered_map<gpi::pc::type::id_t, process_ptr_t> process_map_t;
        typedef std::list<process_ptr_t> process_list_t;
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        void set_state (const state_t new_state);
        state_t get_state (void) const;

        void attach_process (process_ptr_t);
        void detach_process (const gpi::pc::type::process_id_t);
        gpi::pc::type::process_id_t next_process_id ();
        void garbage_collect ();

        mutex_type m_mutex;
        mutable mutex_type m_state_mutex;
        state_t m_state;
        connector_type m_connector;
        gpi::pc::type::process_id_t m_process_id;
        process_map_t m_processes;
        process_list_t m_detached_processes;

        segment_manager_type m_segment_mgr;
      };
    }
  }
}

#include "manager.ipp"

#endif
