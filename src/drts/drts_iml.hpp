#include <iml/client/iml.hpp>
#include <iml/client/scoped_rifd.hpp>
#include <iml/client/virtual_memory.hpp>

#include <iml/vmem/gaspi/pc/type/types.hpp>
#include <iml/vmem/gaspi/pc/type/memory_location.hpp>

#include <we/type/value.hpp>

namespace gspc
{
  namespace iml
  {
    pnet::type::value::value_type remote_iml_gpi_global_memory_range
      (gpi::pc::type::range_t range);
  }
}
