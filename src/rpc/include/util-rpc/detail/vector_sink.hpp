// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
      //! `::boost::iostreams::stream<vector_sink> {sink}` and treating
      //! the latter like a normal `std::ostream`.
      //! \note It is important to flush/destroy the stream and sink
      //! before reading from the underlying vector!
      struct vector_sink : ::boost::iostreams::sink
      {
        vector_sink (std::vector<char>& vector);

        std::streamsize write (char_type const* s, std::streamsize n);

      private:
        std::vector<char>& _vector;
      };
    }
  }
}

#include <util-rpc/detail/vector_sink.ipp>
