#ifndef GPI_SPACE_PC_TYPE_HANDLE_HPP
#define GPI_SPACE_PC_TYPE_HANDLE_HPP 1

#include <iostream>

#include <gpi-space/pc/type/typedefs.hpp>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      struct handle_t
      {
        handle_t ()
          : handle (0)
        {}

        handle_t (handle_id_t h)
          : handle (h)
        {}

        operator handle_id_t () { return handle; }
        bool operator== (const handle_id_t i) const {return handle == i;}

        union
        {
          struct __attribute__((packed))
          {
            handle_id_t cntr : 48;     // those values should be compile time configurable
            handle_id_t ident : 12;
            handle_id_t type : 4;
          } global;
          struct __attribute__((packed))
          {
            handle_id_t cntr : 60;
            handle_id_t type : 4;
          } local;
          handle_id_t handle;
        };
      };

      inline
      std::ostream & operator << (std::ostream & os, const handle_t h)
      {
        std::ios_base::fmtflags saved_flags (os.flags());
        char saved_fill = os.fill (' ');
        std::size_t saved_width = os.width (0);

        os << "0x";
        os.flags (std::ios::hex);
        os.width (sizeof(handle_t)*2);
        os.fill ('0');
        os << h.handle;

        os.flags (saved_flags);
        os.fill (saved_fill);
        os.width (saved_width);

        return os;
      }
    }
  }
}

#endif
