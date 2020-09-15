#pragma once

#include <boost/noncopyable.hpp>

#include <iml/segment_description.hpp>
#include <iml/vmem/gaspi/pc/type/types.hpp>
#include <iml/vmem/gaspi/pc/memory/memory_area.hpp>

#include <util-generic/threadsafe_queue.hpp>

#include <iml/vmem/gaspi_context.hpp>

#include <boost/thread/scoped_thread.hpp>

#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace gpi
{
  namespace pc
  {
    namespace global
    {
      class topology_t;
    }
    namespace memory
    {
      class shm_area_t;

      class manager_t : boost::noncopyable
      {
      public:
        using area_ptr = area_ptr_t;

        manager_t (fhg::iml::vmem::gaspi_context&);
        ~manager_t ();

        void clear ();

        handle_generator_t& handle_generator();

        gpi::pc::type::segment_id_t
        register_memory( const gpi::pc::type::process_id_t creator
                       , std::shared_ptr<shm_area_t> area
                       );
        void unregister_memory ( const gpi::pc::type::process_id_t pid
                               , const gpi::pc::type::segment_id_t
                               );

        void attach_process ( const gpi::pc::type::process_id_t
                            , const gpi::pc::type::segment_id_t
                            );
        void detach_process ( const gpi::pc::type::process_id_t
                            , const gpi::pc::type::segment_id_t
                            );

        int
        remote_alloc ( const gpi::pc::type::segment_id_t
                     , const gpi::pc::type::handle_t
                     , const gpi::pc::type::offset_t
                     , const gpi::pc::type::size_t size
                     , const gpi::pc::type::size_t local_size
                     , const std::string & name
                     );

        gpi::pc::type::handle_t
        alloc ( const gpi::pc::type::process_id_t proc_id
              , const gpi::pc::type::segment_id_t seg_id
              , const gpi::pc::type::size_t size
              , const std::string & name
              , const gpi::pc::type::flags_t flags
              );

        void remote_free(const gpi::pc::type::handle_t hdl);
        void free (const gpi::pc::type::handle_t hdl);
        gpi::pc::type::handle::descriptor_t info (const gpi::pc::type::handle_t hdl) const;
        std::map<std::string, double> get_transfer_costs (const std::list<gpi::pc::type::memory_region_t>&) const;

        void garbage_collect (const gpi::pc::type::process_id_t);

        type::memcpy_id_t memcpy ( type::memory_location_t const & dst
                                 , type::memory_location_t const & src
                                 , const type::size_t amount
                                 );
        void wait (type::memcpy_id_t const&);

        int
        remote_add_memory ( const gpi::pc::type::segment_id_t seg_id
                          , iml::segment_description const& description
                          , unsigned long total_size
                          , global::topology_t& topology
                          );

        gpi::pc::type::segment_id_t
        add_memory ( const gpi::pc::type::process_id_t proc_id
                   , iml::segment_description const& description
                   , unsigned long total_size
                   , global::topology_t& topology
                   );

        int
        remote_del_memory ( const gpi::pc::type::segment_id_t seg_id
                          , global::topology_t& topology
                          );

        void
        del_memory ( const gpi::pc::type::process_id_t proc_id
                   , const gpi::pc::type::segment_id_t seg_id
                   , global::topology_t& topology
                   );

      private:
        typedef std::recursive_mutex mutex_type;
        typedef std::unique_lock<mutex_type> lock_type;
        typedef std::unordered_map< gpi::pc::type::segment_id_t
                                  , area_ptr
                                  > area_map_t;
        typedef std::unordered_map< gpi::pc::type::handle_t
                                  , gpi::pc::type::segment_id_t
                                  > handle_to_segment_t;

        void add_area (area_ptr const &area);
        area_ptr get_area (const gpi::pc::type::segment_id_t);
        area_ptr get_area (const gpi::pc::type::segment_id_t) const;
        area_ptr get_area_by_handle (const gpi::pc::type::handle_t);
        area_ptr get_area_by_handle (const gpi::pc::type::handle_t) const;
        void add_handle ( const gpi::pc::type::handle_t
                        , const gpi::pc::type::segment_id_t
                        );
        void del_handle (const gpi::pc::type::handle_t);
        void unregister_memory (const gpi::pc::type::segment_id_t);

        mutable mutex_type m_mutex;
        area_map_t m_areas;
        handle_to_segment_t m_handle_to_segment;
        fhg::iml::vmem::gaspi_context& _gaspi_context;

        std::mutex _memcpy_task_guard;
        std::size_t _next_memcpy_id;
        fhg::util::interruptible_threadsafe_queue<std::packaged_task<void()>>
          _tasks;
        std::map<std::size_t, std::future<void>> _task_by_id;
        std::vector<std::unique_ptr<boost::strict_scoped_thread<>>>
          _task_threads;
        decltype (_tasks)::interrupt_on_scope_exit _interrupt_task_queue;

        handle_generator_t _handle_generator;
      };
    }
  }
}
