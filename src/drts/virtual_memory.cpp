// mirko.rahn@itwm.fraunhofer.de

#include <drts/drts.hpp>
#include <drts/virtual_memory.hpp>

#include <drts/private/drts_impl.hpp>
#include <drts/private/pimpl.hpp>
#include <drts/private/scoped_vmem_cache.hpp>
#include <drts/private/virtual_memory_impl.hpp>

#include <we/type/value/poke.hpp>

#include <boost/format.hpp>

#include <exception>
#include <sstream>

namespace gspc
{
  scoped_vmem_segment_and_allocation::scoped_vmem_segment_and_allocation
      ( scoped_runtime_system const* const drts
      , vmem::segment_description segment_desc
      , unsigned long size
      )
    : _ ( new scoped_vmem_segment_and_allocation::implementation
            (drts->_->_virtual_memory_api.get(), segment_desc, size)
        )
  {}
  scoped_vmem_segment_and_allocation::scoped_vmem_segment_and_allocation
      ( scoped_runtime_system const* const drts
      , vmem::segment_description segment_desc
      , unsigned long size
      , char const* const data
      )
    : scoped_vmem_segment_and_allocation (drts, segment_desc, size)
  {
    scoped_vmem_cache const buffer
      (*drts->_->_virtual_memory_api, intertwine::vmem::size_t (size));

    auto range ( boost::get<intertwine::vmem::mutable_local_range_t>
                   ( drts->_->_virtual_memory_api->execute_sync
                       ( intertwine::vmem::op::allocate_t
                           (intertwine::vmem::size_t (size), buffer)
                       )
                   )
               );

    char* const content (static_cast<char*> (range.pointer()));
    std::copy (data, data + size, content);

    boost::get<intertwine::vmem::void_t>
      ( drts->_->_virtual_memory_api->execute_sync
          ( intertwine::vmem::op::put_and_release_t
              ( range
              , { _->_data_id
                , { intertwine::vmem::offset_t (0)
                  , intertwine::vmem::size_t {size}
                  }
                }
              )
          )
      );
  }
  PIMPL_DTOR (scoped_vmem_segment_and_allocation)

  std::size_t scoped_vmem_segment_and_allocation::size() const
  {
    return static_cast<std::size_t> (_->_size);
  }
  pnet::type::value::value_type scoped_vmem_segment_and_allocation::global_memory_range
    ( std::size_t const offset
    , std::size_t const size
    ) const
  {
    if ((offset + size) > static_cast<std::size_t> (_->_size))
    {
      throw std::logic_error
        ((boost::format ("slice [%1%, %2%) is outside of allocation")
         % offset % (offset + size)
         ).str()
        );
    }

    pnet::type::value::value_type range;

    {
      std::ostringstream oss;
      boost::archive::binary_oarchive oa (oss);

      oa & _->_data_id;

      pnet::type::value::poke (std::list<std::string> {"handle", "name"}, range, oss.str());
    }
    pnet::type::value::poke (std::list<std::string> {"offset"}, range, offset);
    pnet::type::value::poke (std::list<std::string> {"size"}, range, size);

    return range;
  }
  pnet::type::value::value_type scoped_vmem_segment_and_allocation::global_memory_range() const
  {
    return global_memory_range (0UL, static_cast<std::size_t> (_->_size));
  }
  scoped_vmem_segment_and_allocation::scoped_vmem_segment_and_allocation (scoped_vmem_segment_and_allocation&& other)
    : _ (std::move (other._))
  {}
}
