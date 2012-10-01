#ifndef GPI_SPACE_PC_MEMORY_SFS_AREA_HPP
#define GPI_SPACE_PC_MEMORY_SFS_AREA_HPP

#include <gpi-space/pc/type/segment_type.hpp>

#include <gpi-space/pc/global/itopology.hpp>
#include <gpi-space/pc/memory/memory_area.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class sfs_area_t : public area_t
      {
      public:
        static const type::segment::segment_type area_type = gpi::pc::type::segment::SEG_SFS;

        sfs_area_t ( const gpi::pc::type::process_id_t creator
                   , const std::string & path
                   , const gpi::pc::type::size_t size        // total
                   , const gpi::pc::type::flags_t flags
                   , const std::string & meta_data
                   , gpi::pc::global::itopology_t & topology
                   );

        ~sfs_area_t ();

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

        int get_specific_transfer_tasks ( const gpi::pc::type::memory_location_t src
                                        , const gpi::pc::type::memory_location_t dst
                                        , area_t & dst_area
                                        , gpi::pc::type::size_t amount
                                        , gpi::pc::type::size_t queue
                                        , task_list_t & tasks
                                        );
      private:
        bool is_range_local ( const gpi::pc::type::handle::descriptor_t &
                            , const gpi::pc::type::offset_t begin
                            , const gpi::pc::type::size_t   range_size
                            ) const;

        void *ptr ();

        gpi::pc::type::size_t get_local_size ( const gpi::pc::type::size_t size
                                             , const gpi::pc::type::flags_t flags
                                             ) const;


        void * m_ptr;
        std::string m_path;
        std::string m_meta;

        gpi::pc::type::size_t   m_size;
        gpi::pc::type::offset_t m_min_local_offset;
        gpi::pc::type::offset_t m_max_local_offset;

        gpi::pc::global::itopology_t & m_topology;
      };
    }
  }
}

#endif // GPI_SPACE_PC_MEMORY_AREA_HPP
