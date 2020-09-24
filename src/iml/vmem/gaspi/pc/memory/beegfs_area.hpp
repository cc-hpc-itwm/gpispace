#pragma once

#include <boost/filesystem.hpp>

#include <iml/segment_description.hpp>

#include <iml/vmem/gaspi/pc/global/itopology.hpp>
#include <iml/vmem/gaspi/pc/memory/memory_area.hpp>

#include <util-generic/filesystem_lock_directory.hpp>
#include <util-generic/threadsafe_queue.hpp>

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

        static const int BEEGFS_AREA_VERSION = 0x0001;

        static area_ptr_t create ( iml::beegfs_segment_description const&
                                 , unsigned long total_size
                                 , gpi::pc::global::itopology_t & topology
                                 , bool is_creator
                                 );

        beegfs_area_t ( bool is_creator
                      , const path_t & path
                      , const gpi::pc::type::size_t size        // total
                      , gpi::pc::global::itopology_t & topology
                      );

        ~beegfs_area_t ();

      protected:
        global::itopology_t& global_topology() override;

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

        void open (std::size_t total_size);
        void close();
        double get_transfer_costs ( const gpi::pc::type::memory_region_t&
                                  , const gpi::rank_t
                                  ) const override;

        fhg::util::threadsafe_queue<int> _fds;
        struct lock_file_helper : fhg::util::filesystem_lock_directory
        {
          lock_file_helper (beegfs_area_t&);
        };
        bool _is_creator;
        //! \todo boost::optional with move-assignment support
        std::unique_ptr<lock_file_helper> _lock_file;
        path_t m_path;
        int    m_version;

        gpi::pc::global::itopology_t & m_topology;
      };
    }
  }
}
