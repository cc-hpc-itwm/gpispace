// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

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
          error_t (errc ec, std::string const& det="")
            : code (ec)
            , detail (det)
          {}
        private:
          friend class ::boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, unsigned int /*version*/)
          {
            ar & code;
            ar & detail;
          }
        };

        using message_t = error_t;
      }
    }
  }
}
