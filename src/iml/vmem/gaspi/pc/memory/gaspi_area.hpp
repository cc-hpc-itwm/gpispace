#pragma once

#include <iml/segment_description.hpp>
#include <iml/vmem/gaspi/gpi/gaspi.hpp>

#include <iml/vmem/gaspi/pc/memory/memory_area.hpp>
#include <iml/vmem/gaspi/pc/memory/handle_buffer.hpp>
#include <iml/vmem/gaspi/pc/memory/handle_generator.hpp>

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
        typedef fhg::util::threadsafe_queue<handle_buffer_t> handle_pool_t;

        static area_ptr_t create ( iml::gaspi_segment_description const&
                                 , unsigned long total_size
                                 , gpi::pc::global::itopology_t & topology
                                 , handle_generator_t&
                                 , fhg::iml::vmem::gaspi_context&
                                 );

      protected:
        gaspi_area_t ( gpi::pc::global::itopology_t & topology
                     , handle_generator_t&
                     , fhg::iml::vmem::gaspi_context&
                     , fhg::iml::vmem::gaspi_timeout&
                     , type::size_t memory_size
                     , type::size_t num_com_buffers
                     , type::size_t com_buffer_size
                     );

        virtual global::itopology_t& global_topology() override;

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
