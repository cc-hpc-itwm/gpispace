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

#include <fhgcom/header.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace fhg
{
  namespace com
  {
    struct message_t
    {
      message_t () = default;

      explicit
      message_t (std::size_t len)
        : data(len)
      {}

      message_t (const char * d, std::size_t len)
        : data(d, d+len)
      {}

      template <typename Iterator>
      message_t (Iterator begin, Iterator end)
        : data(begin, end)
      {}

      void resize (const std::size_t n)
      {
        data.resize (n);
        header.length = n;
      }

      void resize ()
      {
        data.resize (header.length);
      }

      void assign (std::string const& x)
      {
        data.assign(x.begin(), x.end());
        header.length = data.size ();
      }

      const char * buf () const { return &data[0]; }
      std::size_t size () const { return data.size(); }

      p2p::header_t header;
      std::vector<char> data;
    };
  }
}
