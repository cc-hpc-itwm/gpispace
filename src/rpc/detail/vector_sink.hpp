// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <boost/iostreams/concepts.hpp>

#include <ios>
#include <vector>

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      //! A Boost.IOStreams sink which inserts into a std::vector.
      //! Use by creating a `std::vector`, a `vector_sink {vec}`, a
      //! `boost::iostreams::stream<vector_sink> {sink}` and treating
      //! the latter like a normal `std::ostream`.
      //! \note It is important to flush/destroy the stream and sink
      //! before reading from the underlying vector!
      struct vector_sink : boost::iostreams::sink
      {
        vector_sink (std::vector<char>& vector);

        std::streamsize write (char_type const* s, std::streamsize n);

      private:
        std::vector<char>& _vector;
      };
    }
  }
}

#include <rpc/detail/vector_sink.ipp>
