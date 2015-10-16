#pragma once

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

        static area_ptr_t create ( fhg::log::Logger&
                                 , type::name_t const&
                                 , type::size_t
                                 , handle_generator_t&
                                 );

        shm_area_t ( fhg::log::Logger&
                   , const gpi::pc::type::process_id_t creator
                   , const std::string & name // == path
                   , const gpi::pc::type::size_t size
                   , const gpi::pc::type::flags_t flags
                   , handle_generator_t&
                   );

        ~shm_area_t ();
      protected:
        virtual Arena_t grow_direction (const gpi::pc::type::flags_t) const override;
        int get_type_id () const;

        virtual void alloc_hook (const gpi::pc::type::handle::descriptor_t &) override{}
        virtual void  free_hook (const gpi::pc::type::handle::descriptor_t &) override{}

        virtual int get_specific_transfer_tasks ( const gpi::pc::type::memory_location_t src
                                        , const gpi::pc::type::memory_location_t dst
                                        , area_t & dst_area
                                        , gpi::pc::type::size_t amount
                                        , gpi::pc::type::size_t queue
                                        , task_list_t & tasks
                                        ) override;
      private:
        virtual void *raw_ptr (gpi::pc::type::offset_t off) override;

        virtual bool is_range_local ( const gpi::pc::type::handle::descriptor_t &
                            , const gpi::pc::type::offset_t a
                            , const gpi::pc::type::offset_t b
                            ) const override;
        virtual gpi::pc::type::size_t get_local_size ( const gpi::pc::type::size_t size
                                             , const gpi::pc::type::flags_t flgs
                                             ) const override;


        double get_transfer_costs ( const gpi::pc::type::memory_region_t&
                                  , const gpi::rank_t
                                  ) const override;

        static bool unlink_after_open (const gpi::pc::type::flags_t);
        static bool unlink_after_close (const gpi::pc::type::flags_t);

        void *m_ptr;
        std::string m_path;
      };
    }
  }
}
