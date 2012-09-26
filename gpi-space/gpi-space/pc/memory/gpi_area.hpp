#ifndef GPI_SPACE_PC_MEMORY_GPI_AREA_HPP
#define GPI_SPACE_PC_MEMORY_GPI_AREA_HPP

#include <gpi-space/pc/memory/memory_area.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class gpi_area_t : public area_t
      {
      public:
        static const int area_type = 1;

        gpi_area_t ( const gpi::pc::type::id_t id
                   , const gpi::pc::type::process_id_t creator
                   , const std::string & name
                   , const gpi::pc::type::size_t per_node_size
                   , const gpi::pc::type::flags_t flags
                   , void * dma_ptr
                   );

        ~gpi_area_t ();

      protected:
        int get_type_id () const;

        void check_bounds ( const gpi::pc::type::handle::descriptor_t &
                          , const gpi::pc::type::offset_t start
                          , const gpi::pc::type::size_t   amount
                          ) const;

        bool is_allowed_to_attach (const gpi::pc::type::process_id_t) const;
        grow_direction_t grow_direction (const gpi::pc::type::flags_t) const;

        void alloc_hook (const gpi::pc::type::handle::descriptor_t &);
        void  free_hook (const gpi::pc::type::handle::descriptor_t &);
      private:
        bool is_range_local ( const gpi::pc::type::handle::descriptor_t &
                            , const gpi::pc::type::offset_t begin
                            , const gpi::pc::type::size_t   range_size
                            ) const;

        gpi::pc::type::offset_t
        handle_to_global_offset ( const gpi::pc::type::offset_t
                                , const gpi::pc::type::size_t per_node_size
                                ) const;

        void *ptr ();

        gpi::pc::type::size_t get_local_size ( const gpi::pc::type::size_t size
                                             , const gpi::pc::type::flags_t flags
                                             ) const;


        void * m_ptr;
        gpi::pc::type::size_t m_total_memsize;
        gpi::pc::type::offset_t m_min_local_offset;
        gpi::pc::type::offset_t m_max_local_offset;
      };
    }
  }
}

#endif // GPI_SPACE_PC_MEMORY_AREA_HPP
