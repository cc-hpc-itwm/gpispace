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

#include <time.h>
#include <sys/time.h>

// serialization
#include <boost/serialization/nvp.hpp>

#include <gpi-space/pc/type/typedefs.hpp>

namespace boost
{
  namespace serialization
  {
    template <typename Archive>
    void serialize (Archive & ar, timeval & tv, const unsigned int /*version*/)
    {
      ar & tv.tv_sec;
      ar & tv.tv_usec;
    }
  }
}

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      struct time_stamp_t
      {
        static const int TOUCH_ACCESSED = 0x01;
        static const int TOUCH_MODIFIED = 0x02;
        static const int TOUCH_ALL = (TOUCH_ACCESSED | TOUCH_MODIFIED);

        gpi::pc::type::time_t created;
        gpi::pc::type::time_t accessed;
        gpi::pc::type::time_t modified;

        time_stamp_t ()
          : created (now())
          , accessed (created)
          , modified (created)
        {}

        void touch ()
        {
          touch (TOUCH_ACCESSED | TOUCH_MODIFIED);
        }

        void touch (const int what)
        {
          touch (what, 0);
        }

        void touch (const int what, const gpi::pc::type::time_t tstamp)
        {
          const gpi::pc::type::time_t n ( (tstamp != 0) ? tstamp : now());

          if (what & TOUCH_ACCESSED)
          {
            accessed = n;
          }

          if (what & TOUCH_MODIFIED)
          {
            modified = n;
          }
        }

        static gpi::pc::type::time_t now ()
        {
          return time(nullptr);
        }

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( created );
            ar & BOOST_SERIALIZATION_NVP( accessed );
            ar & BOOST_SERIALIZATION_NVP( modified );
          }
      };
    }
  }
}
