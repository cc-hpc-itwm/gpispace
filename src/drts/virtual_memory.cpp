#include <drts/virtual_memory.hpp>

#include <iml/vmem/gaspi/pc/type/types.hpp>

#include <we/type/value/poke.hpp>
#include <we/type/range.hpp>

#include <boost/range/adaptor/map.hpp>

#include <sstream>

namespace gspc
{
  vmem_allocation::vmem_allocation (iml_client::vmem_allocation alloc)
    : _alloc (std::move (alloc))
  {}
  std::size_t vmem_allocation::size() const
  {
    return _alloc.size();
  }

  pnet::type::value::value_type vmem_allocation::global_memory_range
    ( std::size_t const offset
    , std::size_t const size
    ) const
  {
    if ((offset + size) > _alloc.size())
    {
      throw std::logic_error
        ((boost::format ("slice [%1%, %2%) is outside of allocation")
         % offset % (offset + size)
         ).str()
        );
    }

    gpi::pc::type::range_t const iml_range
      (_alloc.global_memory_range (offset, size));

    pnet::type::value::value_type range;
    {
      std::ostringstream oss;

      oss << "0x";
      oss.flags (std::ios::hex);
      oss.width (18);
      oss.fill ('0');
      oss << iml_range.handle;

      pnet::type::value::poke ( std::list<std::string> {"handle", "name"}
                              , range
                              , oss.str()
                              );
    }
    pnet::type::value::poke ( std::list<std::string> {"offset"}
                            , range
                            , iml_range.offset);
    pnet::type::value::poke ( std::list<std::string> {"size"}
                            , range
                            , iml_range.size
                            );

    return range;
  }
  pnet::type::value::value_type vmem_allocation::global_memory_range() const
  {
    return global_memory_range (0UL, _alloc.size());
  }
}
