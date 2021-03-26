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

#include <buffer.hpp>
#include <queue.hpp>
#include <stdexcept>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sstream>

template<typename T>
class writer
{
public:
  typedef fhg::buffer::type<T> buffer_type;
  typedef fhg::thread::queue<buffer_type> queue_type;

private:
  queue_type& _queue_empty;
  queue_type& _queue_full;
  FILE* _file;

public:
  writer ( queue_type& queue_empty
         , queue_type& queue_full
         , FILE* file
         )
    : _queue_empty (queue_empty)
    , _queue_full (queue_full)
    , _file (file)
  {}

  void operator() ()
  {
    buffer_type buffer;

    do
    {
      buffer = _queue_full.get();

      if (buffer.count() != fwrite ( buffer.begin()
                                   , sizeof (T)
                                   , buffer.count()
                                   , _file
                                   )
         )
      {
        const int ec (errno);

        std::ostringstream oss;

        oss << "write failed: ec = " << ec
            << ", reason = " << strerror (ec);

        throw std::runtime_error (oss.str());
      }

      _queue_empty.put (buffer);
    }
    while (buffer.count() != 0);
  }
};
