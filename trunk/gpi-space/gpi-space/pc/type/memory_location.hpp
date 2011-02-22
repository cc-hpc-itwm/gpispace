#ifndef GPI_SPACE_PC_TYPE_MEMORY_LOCATION_HPP
#define GPI_SPACE_PC_TYPE_MEMORY_LOCATION_HPP 1

#include <iostream>
#include <string>

#include <boost/lexical_cast.hpp>
// serialization
#include <boost/serialization/nvp.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/handle.hpp>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      struct memory_location_t
      {
        gpi::pc::type::handle_id_t handle;
        gpi::pc::type::offset_t    offset;

      private:
        friend class boost::serialization::access;
        template<typename Archive>
        void serialize (Archive & ar, const unsigned int /*version*/)
        {
          ar & BOOST_SERIALIZATION_NVP( handle );
          ar & BOOST_SERIALIZATION_NVP( offset );
        }
      };

      inline
      std::ostream & operator<< ( std::ostream & os
                                , const memory_location_t & loc
                                )
      {
        return os << handle_t(loc.handle) << "+" << loc.offset;
      }

      inline
      std::istream & operator>> ( std::istream & is
                                , memory_location_t & loc
                                )
      {
        // read a string
        std::string tmp;
        is >> tmp;

        std::string::size_type split_pos (tmp.find('+'));
        loc.handle =
            boost::lexical_cast<handle_t>(tmp.substr( 0
                                                    , split_pos
                                                    )
                                         );
        if (split_pos != std::string::npos)
        {
          loc.offset =
              boost::lexical_cast<offset_t>
                (tmp.substr ( split_pos+1
                            , tmp.size() - split_pos
                            )
                );
        }
        else
        {
          loc.offset = 0;
        }
        return is;
      }
    }
  }
}

#endif
