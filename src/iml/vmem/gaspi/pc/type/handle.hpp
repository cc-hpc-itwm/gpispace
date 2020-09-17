#pragma once

#include <iostream>
#include <stdexcept>

#include <boost/lexical_cast.hpp>
#include <boost/serialization/nvp.hpp>

#include <iml/vmem/gaspi/pc/type/types.hpp>

#include <functional>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      struct handle_t
      {
      private:
        static const int total_bits = (sizeof(handle_id_t)*8);
        static const int ident_bits = 12;
        static const int global_count_bits = (total_bits - ident_bits);
        static const int string_length = (2+total_bits/4); // 0x...
        template <size_t Bits>
          void check_for_overflow (const type::handle_id_t part)
        {
          if (part > ((1UL << Bits) - 1))
          {
            throw std::runtime_error ("handle would overflow");
          }
        }

      public:
        handle_t (handle_id_t global_id, handle_id_t counter)
        {
          check_for_overflow<ident_bits> (global_id);
          check_for_overflow<global_count_bits> (counter);

          ident = global_id;
          cntr = counter;
        }
        handle_t (handle_id_t id)
          : handle (id)
        {}

        operator handle_id_t () const { return handle; }
        bool operator== (const handle_id_t o) const {return handle == o;}
        bool operator<  (const handle_id_t o) const {return handle < o; }

        //! For serialization only.
        handle_t() = default;

        friend std::ostream& operator<< (std::ostream&, handle_t const&);
        friend std::istream& operator>> (std::istream&, handle_t&);
        friend std::hash<gpi::pc::type::handle_t>;

      private:
        union
        {
          struct // == sizeof(handle_id_t)
          {
            handle_id_t cntr  : global_count_bits;
            handle_id_t ident : ident_bits;
          };
          handle_id_t handle;
        };

        friend class boost::serialization::access;
        template<typename Archive>
        void serialize (Archive & ar, const unsigned int /*version*/)
        {
          ar & BOOST_SERIALIZATION_NVP( handle );
        }
      };

      inline
      std::ostream & operator << (std::ostream & os, const handle_t& h)
      {
        const std::ios_base::fmtflags saved_flags (os.flags());
        const char saved_fill = os.fill (' ');
        const std::size_t saved_width = os.width (0);

        os << "0x";
        os.flags (std::ios::hex);
        os.width (handle_t::string_length - 2);
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

        is.flags (saved_flags);
        return is;
      }
    }
  }
}

namespace std
{
  template<> struct hash<gpi::pc::type::handle_t>
  {
    size_t operator()(gpi::pc::type::handle_t const& x) const
    {
      return std::hash<gpi::pc::type::handle_id_t>() (x.handle);
    }
  };
}
