#ifndef GPI_SPACE_PC_MEMORY_SFS_AREA_HPP
#define GPI_SPACE_PC_MEMORY_SFS_AREA_HPP

#include <boost/filesystem.hpp>

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
        typedef boost::filesystem::path path_t;

        static const type::segment::segment_type area_type = gpi::pc::type::segment::SEG_SFS;
        static const int SFS_VERSION = 0x0001;

        // cleanup a file segment
        static void cleanup (path_t const & path);

        sfs_area_t ( const gpi::pc::type::process_id_t creator
                   , const path_t & path
                   , const gpi::pc::type::size_t size        // total
                   , const gpi::pc::type::flags_t flags
                   , gpi::pc::global::itopology_t & topology
                   );

        ~sfs_area_t ();

        int save_state (boost::system::error_code &ec);
        int open (boost::system::error_code & ec);
        int close (boost::system::error_code & ec);

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

        void *raw_ptr (gpi::pc::type::offset_t off);

        gpi::pc::type::size_t get_local_size ( const gpi::pc::type::size_t size
                                             , const gpi::pc::type::flags_t flags
                                             ) const;

        gpi::pc::type::size_t
        read_from_impl ( gpi::pc::type::offset_t off
                       , void *buffer
                       , gpi::pc::type::size_t amount
                       );

        gpi::pc::type::size_t
        write_to_impl ( gpi::pc::type::offset_t off
                      , const void *buffer
                      , gpi::pc::type::size_t amount
                      );

        bool initialize ( path_t const & path
                        , const gpi::pc::type::size_t size
                        , boost::system::error_code &ec
                        );

        void * m_ptr;
        int    m_fd;
        path_t m_path;
        int    m_version;

        gpi::pc::type::size_t   m_size;

        gpi::pc::global::itopology_t & m_topology;
      };
    }
  }
}

#endif // GPI_SPACE_PC_MEMORY_AREA_HPP
