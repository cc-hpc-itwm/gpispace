#ifndef GPI_SPACE_PC_TYPE_HANDLE_HPP
#define GPI_SPACE_PC_TYPE_HANDLE_HPP 1

#include <iostream>
#include <stdexcept>

#include <boost/lexical_cast.hpp>

#include <gpi-space/pc/type/typedefs.hpp>

// TODO: move this define to a better place
#if !defined(HANDLE_IDENT_BITS)
#  define HANDLE_IDENT_BITS 12
#endif

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      template <size_t Bits>
      void check_for_overflow (const gpi::pc::type::handle_id_t part)
      {
        if (part > ((1UL << Bits) - 1))
        {
          throw std::runtime_error ("handle would overflow");
        }
      }

      struct handle_t
      {
        static const int total_bits = (sizeof(handle_id_t)*8);
        static const int ident_bits = HANDLE_IDENT_BITS;
        static const int typec_bits = 4;
        static const int global_count_bits = (total_bits - ident_bits - typec_bits);
        static const int local_count_bits = (total_bits - typec_bits);

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
            union
            {
              struct // == sizeof(handle_id_t)
              {
                handle_id_t cntr  : global_count_bits;
                handle_id_t ident : ident_bits;
                int _pad  : typec_bits;
              } global;
              struct // == sizeof(handle_id_t)
              {
                handle_id_t cntr : local_count_bits;
                int _pad  : typec_bits;
              } local;
              struct // == sizeof(handle_id_t)
              {
                handle_id_t _pad : (total_bits - typec_bits);
                int type : typec_bits;
              };
            };
          };
          handle_id_t handle;
        };
      };

      inline
      bool is_valid (const handle_t & hdl)
      {
        switch (hdl.type)
        {
          case 1:
          case 2:
          case 3:
            return true;
        }
        return false;
      }

      inline
      void validate (const handle_t & hdl)
      {
        if (! is_valid (hdl))
        {
          throw std::invalid_argument("invalid handle");
        }
      }

      inline
      std::ostream & operator << (std::ostream & os, const handle_t h)
      {
        const std::ios_base::fmtflags saved_flags (os.flags());
        const char saved_fill = os.fill (' ');
        const std::size_t saved_width = os.width (0);

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

      inline
      std::istream & operator>> ( std::istream & is
                                , handle_t & h
                                )
      {
        const std::ios_base::fmtflags saved_flags (is.flags());
        is.flags (std::ios::hex);
        is >> h.handle;

        validate (h);

        is.flags (saved_flags);
        return is;
      }
    }
  }
}

#endif
