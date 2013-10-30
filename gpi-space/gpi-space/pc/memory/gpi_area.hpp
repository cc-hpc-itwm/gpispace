#ifndef GPI_SPACE_PC_MEMORY_GPI_AREA_HPP
#define GPI_SPACE_PC_MEMORY_GPI_AREA_HPP

#include <gpi-space/pc/type/segment_type.hpp>
#include <gpi-space/pc/memory/memory_area.hpp>
#include <gpi-space/pc/memory/handle_buffer.hpp>
#include <gpi-space/pc/memory/memory_buffer_pool.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class gpi_area_t : public area_t
      {
      public:
        static const type::segment::segment_type area_type = gpi::pc::type::segment::SEG_GPI;

        typedef buffer_pool_t<handle_buffer_t> handle_pool_t;

        static area_ptr_t create (std::string const &url);

      protected:
        gpi_area_t ( const gpi::pc::type::process_id_t creator
                   , const std::string & name
                   , const gpi::pc::type::size_t per_node_size
                   , const gpi::pc::type::flags_t flags
                   , void * dma_ptr
                   );

        void check_bounds ( const gpi::pc::type::handle::descriptor_t &
                          , const gpi::pc::type::offset_t start
                          , const gpi::pc::type::size_t   amount
                          ) const;

        bool is_allowed_to_attach (const gpi::pc::type::process_id_t) const;
        grow_direction_t grow_direction (const gpi::pc::type::flags_t) const;

        void alloc_hook (const gpi::pc::type::handle::descriptor_t &);
        void  free_hook (const gpi::pc::type::handle::descriptor_t &);

        int get_specific_transfer_tasks ( const gpi::pc::type::memory_location_t src
                                        , const gpi::pc::type::memory_location_t dst
                                        , area_t & dst_area
                                        , gpi::pc::type::size_t amount
                                        , gpi::pc::type::size_t queue
                                        , task_list_t & tasks
                                        );

        int get_send_tasks ( area_t & src_area
                           , const gpi::pc::type::memory_location_t src
                           , const gpi::pc::type::memory_location_t dst
                           , gpi::pc::type::size_t amount
                           , gpi::pc::type::size_t queue
                           , task_list_t & tasks
                           );

        int get_recv_tasks ( area_t & dst_area
                           , const gpi::pc::type::memory_location_t dst
                           , const gpi::pc::type::memory_location_t src
                           , gpi::pc::type::size_t amount
                           , gpi::pc::type::size_t queue
                           , task_list_t & tasks
                           );
      private:
        void init ();

        bool is_range_local ( const gpi::pc::type::handle::descriptor_t &
                            , const gpi::pc::type::offset_t begin
                            , const gpi::pc::type::size_t   range_size
                            ) const;

        gpi::pc::type::offset_t
        handle_to_global_offset ( const gpi::pc::type::offset_t
                                , const gpi::pc::type::size_t per_node_size
                                ) const;

        void *raw_ptr (gpi::pc::type::offset_t off);

        gpi::pc::type::size_t get_local_size ( const gpi::pc::type::size_t size
                                             , const gpi::pc::type::flags_t flags
                                             ) const;


        void * m_ptr;
        gpi::pc::type::size_t m_total_memsize;
        gpi::pc::type::offset_t m_min_local_offset;
        gpi::pc::type::offset_t m_max_local_offset;

        handle_pool_t m_com_handles; // local allocations that can be used to transfer data

        gpi::pc::type::size_t m_num_com_buffers;
        gpi::pc::type::size_t m_com_buffer_size;
      };
    }
  }
}

#endif // GPI_SPACE_PC_MEMORY_AREA_HPP
