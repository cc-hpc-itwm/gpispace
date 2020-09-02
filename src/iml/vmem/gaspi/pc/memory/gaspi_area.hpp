#pragma once

#include <iml/vmem/gaspi/gpi/gaspi.hpp>

#include <iml/vmem/gaspi/pc/type/segment_type.hpp>
#include <iml/vmem/gaspi/pc/memory/memory_area.hpp>
#include <iml/vmem/gaspi/pc/memory/handle_buffer.hpp>

#include <iml/vmem/gaspi/pc/global/itopology.hpp>

#include <iml/vmem/gaspi_context.hpp>

#include <util-generic/threadsafe_queue.hpp>

namespace gpi
{
  namespace api
  {
    class gaspi_t;
  }
  namespace pc
  {
    namespace memory
    {
      class gaspi_area_t : public area_t
      {
      public:
        static const type::segment::segment_type area_type = gpi::pc::type::segment::SEG_GASPI;

        typedef fhg::util::threadsafe_queue<handle_buffer_t> handle_pool_t;

        static area_ptr_t create ( std::string const &url
                                 , gpi::pc::global::itopology_t & topology
                                 , handle_generator_t&
                                 , fhg::iml::vmem::gaspi_context&
                                 , type::id_t owner
                                 );

      protected:
        gaspi_area_t ( const gpi::pc::type::process_id_t creator
                     , const std::string & name
                     , const gpi::pc::type::flags_t flags
                     , gpi::pc::global::itopology_t & topology
                     , handle_generator_t&
                     , fhg::iml::vmem::gaspi_context&
                     , fhg::iml::vmem::gaspi_timeout&
                     , type::size_t memory_size
                     , type::size_t num_com_buffers
                     , type::size_t com_buffer_size
                     );

        virtual bool is_allowed_to_attach (const gpi::pc::type::process_id_t) const override;
        virtual iml_client::vmem::dtmmgr::Arena_t grow_direction (const gpi::pc::type::flags_t) const override;

        virtual void alloc_hook (const gpi::pc::type::handle::descriptor_t &) override;
        virtual void  free_hook (const gpi::pc::type::handle::descriptor_t &) override;

        virtual std::packaged_task<void()> get_send_task
          ( area_t & src_area
          , const gpi::pc::type::memory_location_t src
          , const gpi::pc::type::memory_location_t dst
          , gpi::pc::type::size_t amount
          ) override;

        virtual std::packaged_task<void()> get_recv_task
          ( area_t & dst_area
          , const gpi::pc::type::memory_location_t dst
          , const gpi::pc::type::memory_location_t src
          , gpi::pc::type::size_t amount
          ) override;

      private:
        virtual bool is_range_local ( const gpi::pc::type::handle::descriptor_t &
                            , const gpi::pc::type::offset_t begin
                            , const gpi::pc::type::size_t   range_size
                            ) const override;

        gpi::pc::type::offset_t
        handle_to_global_offset ( const gpi::pc::type::offset_t
                                , const gpi::pc::type::size_t per_node_size
                                ) const;

        virtual void *raw_ptr (gpi::pc::type::offset_t off) override;

        virtual gpi::pc::type::size_t get_local_size ( const gpi::pc::type::size_t size
                                             , const gpi::pc::type::flags_t flags
                                             ) const override;

        double get_transfer_costs ( const gpi::pc::type::memory_region_t&
                                  , const gpi::rank_t
                                  ) const override;

        fhg::iml::vmem::gaspi_context& _gaspi_context;
        api::gaspi_t _gaspi;

        void * m_ptr;

        handle_pool_t m_com_handles; // local allocations that can be used to transfer data

        gpi::pc::type::size_t m_num_com_buffers;
        gpi::pc::type::size_t m_com_buffer_size;

        gpi::pc::global::itopology_t & _topology;
      };
    }
  }
}
