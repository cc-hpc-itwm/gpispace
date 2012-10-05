#ifndef GPI_SPACE_PC_MEMORY_SHM_AREA_HPP
#define GPI_SPACE_PC_MEMORY_SHM_AREA_HPP

#include <gpi-space/pc/type/segment_type.hpp>
#include <gpi-space/pc/memory/memory_area.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class shm_area_t : public virtual area_t
      {
      public:
        static const type::segment::segment_type area_type = gpi::pc::type::segment::SEG_SHM;

        shm_area_t ( const gpi::pc::type::process_id_t creator
                   , const std::string & name // == path
                   , const gpi::pc::type::size_t size
                   , const gpi::pc::type::flags_t flags
                   );

        ~shm_area_t ();
      protected:
        bool is_allowed_to_attach (const gpi::pc::type::process_id_t) const;
        grow_direction_t grow_direction (const gpi::pc::type::flags_t) const;
        int get_type_id () const;

        void check_bounds ( const gpi::pc::type::handle::descriptor_t &
                          , const gpi::pc::type::offset_t start
                          , const gpi::pc::type::size_t   amount
                          ) const;

        void alloc_hook (const gpi::pc::type::handle::descriptor_t &){}
        void  free_hook (const gpi::pc::type::handle::descriptor_t &){}

        int get_specific_transfer_tasks ( const gpi::pc::type::memory_location_t src
                                        , const gpi::pc::type::memory_location_t dst
                                        , area_t & dst_area
                                        , gpi::pc::type::size_t amount
                                        , gpi::pc::type::size_t queue
                                        , task_list_t & tasks
                                        );
      private:
        void *raw_ptr (gpi::pc::type::offset_t off);

        bool is_range_local ( const gpi::pc::type::handle::descriptor_t &
                            , const gpi::pc::type::offset_t a
                            , const gpi::pc::type::offset_t b
                            ) const;
        gpi::pc::type::size_t get_local_size ( const gpi::pc::type::size_t size
                                             , const gpi::pc::type::flags_t flgs
                                             ) const;


        static bool unlink_after_open (const gpi::pc::type::flags_t);
        static bool unlink_after_close (const gpi::pc::type::flags_t);

        void *m_ptr;
        std::string m_path;
      };
    }
  }
}

#endif // GPI_SPACE_PC_MEMORY_SHM_AREA_HPP
