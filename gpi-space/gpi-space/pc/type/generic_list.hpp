#ifndef GPI_SPACE_PC_TYPE_GENERIC_LIST_HPP
#define GPI_SPACE_PC_TYPE_GENERIC_LIST_HPP 1

#include <gpi-space/pc/type/typedefs.hpp>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      template <typename Elem>
      struct generic_list_t
      {
        typedef Elem element_type;
        gpi::pc::type::size_t count;
        element_type item[0];
      };
    }
  }
}

#endif
