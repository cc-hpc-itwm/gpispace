// mirko.rahn@itwm.fraunhofer.de

#ifndef SHARE_EXAMPLE_STREAM_PRODUCER_HPP
#define SHARE_EXAMPLE_STREAM_PRODUCER_HPP

#include <we/type/value.hpp>

#include <gpi-space/pc/client/api.hpp>
#include <gpi-space/pc/segment/segment.hpp>
#include <gpi-space/pc/type/handle.hpp>

#include <sdpa/types.hpp>

#include <drts/client.fwd.hpp>
#include <drts/virtual_memory.fwd.hpp>

#include <boost/filesystem.hpp>

#include <string>
#include <unordered_set>

namespace stream
{
  class producer
  {
  public:
    producer ( std::unique_ptr<gpi::pc::client::api_t> const& virtual_memory
             , gspc::vmem_allocation const& buffer
             , gspc::vmem_allocation const& meta
             , unsigned long num_slot
             , unsigned long size_slot
             , gspc::client*
             , sdpa::job_id_t const&
             , std::string const& place_name
             );

    // data.size must not be greater than the slot size
    void produce (std::string const& data);

  private:
    std::unique_ptr<gpi::pc::client::api_t> const& _virtual_memory;
    gspc::vmem_allocation const& _buffer;
    gspc::vmem_allocation const& _meta;
    unsigned long const _num_slot;
    unsigned long const _size_slot;
    gspc::client* _client;
    sdpa::job_id_t const _job_id;
    std::string const _place_name;

    class scoped_allocation
    {
    public:
      scoped_allocation ( std::unique_ptr<gpi::pc::client::api_t> const&
                        , std::string const& description
                        , unsigned long size
                        );
      ~scoped_allocation();

      operator gpi::pc::type::handle_t const& () const
      {
        return _handle;
      }

    private:
      std::unique_ptr<gpi::pc::client::api_t> const& _virtual_memory;
      gpi::pc::type::handle_t const _segment;
      gpi::pc::type::handle_t const _handle;
    };
    scoped_allocation const _flags;
    scoped_allocation const _update;
    scoped_allocation const _data;

    std::unordered_set<unsigned long> _free_slots;
  };
}

#endif
