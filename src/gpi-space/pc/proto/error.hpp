#ifndef GPI_SPACE_PC_PROTO_ERROR_HPP
#define GPI_SPACE_PC_PROTO_ERROR_HPP 1

#include <string>

// serialization
#include <boost/serialization/nvp.hpp>

#include <gpi-space/pc/type/typedefs.hpp>

namespace gpi
{
  namespace pc
  {
    namespace proto
    {
      namespace error
      {
        enum errc
          {
            success = 0,
            bad_request = 10,
            out_of_memory = 30,
          };

        struct error_t
        {
          errc code;
          std::string detail;

          error_t ()
            : code (success)
            , detail ("sucess")
          {}

          explicit
          error_t (errc ec, std::string const &det="")
            : code (ec)
            , detail (det)
          {}
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( code );
            ar & BOOST_SERIALIZATION_NVP( detail );
          }
        };

        typedef error_t message_t;
      }
    }
  }
}

#endif
