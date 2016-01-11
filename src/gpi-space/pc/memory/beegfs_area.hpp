#pragma once

#include <boost/filesystem.hpp>

#include <gpi-space/pc/type/segment_type.hpp>

#include <gpi-space/pc/global/itopology.hpp>
#include <gpi-space/pc/memory/memory_area.hpp>

#include <util-generic/filesystem_lock_directory.hpp>
#include <fhg/util/thread/queue.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class beegfs_area_t : public area_t
      {
      public:
        typedef boost::filesystem::path path_t;

        static const type::segment::segment_type area_type = gpi::pc::type::segment::SEG_BEEGFS;
        static const int BEEGFS_AREA_VERSION = 0x0001;

        static area_ptr_t create ( fhg::log::Logger&
                                 , std::string const &url
                                 , gpi::pc::global::itopology_t & topology
                                 , handle_generator_t&
                                 , type::id_t owner
                                 );

        beegfs_area_t ( fhg::log::Logger&
                      , const gpi::pc::type::process_id_t creator
                      , const path_t & path
                      , const gpi::pc::type::size_t size        // total
                      , const gpi::pc::type::flags_t flags
                      , gpi::pc::global::itopology_t & topology
                      , handle_generator_t&
                      );

        ~beegfs_area_t ();

      protected:
        int get_type_id () const;

        virtual bool is_allowed_to_attach (const gpi::pc::type::process_id_t) const override;
        virtual gspc::vmem::dtmmgr::Arena_t grow_direction (const gpi::pc::type::flags_t) const override;

        virtual void alloc_hook (const gpi::pc::type::handle::descriptor_t &) override;
        virtual void  free_hook (const gpi::pc::type::handle::descriptor_t &) override;

      private:
        virtual bool is_range_local ( const gpi::pc::type::handle::descriptor_t &
                            , const gpi::pc::type::offset_t begin
                            , const gpi::pc::type::size_t   range_size
                            ) const override;

        virtual void *raw_ptr (gpi::pc::type::offset_t off) override;

        virtual gpi::pc::type::size_t get_local_size ( const gpi::pc::type::size_t size
                                             , const gpi::pc::type::flags_t flags
                                             ) const override;

        virtual gpi::pc::type::size_t
        read_from_impl ( gpi::pc::type::offset_t off
                       , void *buffer
                       , gpi::pc::type::size_t amount
                       ) override;

        virtual gpi::pc::type::size_t
        write_to_impl ( gpi::pc::type::offset_t off
                      , const void *buffer
                      , gpi::pc::type::size_t amount
                      ) override;

        std::string version_string() const;

        void open();
        void close();
        double get_transfer_costs ( const gpi::pc::type::memory_region_t&
                                  , const gpi::rank_t
                                  ) const override;

        fhg::thread::queue<int> _fds;
        struct lock_file_helper : fhg::util::filesystem_lock_directory
        {
          lock_file_helper (beegfs_area_t&);
        };
        //! \todo boost::optional with move-assignment support
        std::unique_ptr<lock_file_helper> _lock_file;
        path_t m_path;
        int    m_version;

        gpi::pc::global::itopology_t & m_topology;
      };
    }
  }
}
