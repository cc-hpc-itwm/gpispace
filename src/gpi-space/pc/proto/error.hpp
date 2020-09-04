// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

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
