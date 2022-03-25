// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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
#include <stdio.h>

template<typename T>
class reader
{
public:
  typedef fhg::buffer::type<T> buffer_type;
  typedef fhg::thread::queue<buffer_type> queue_type;

private:
  queue_type& _queue_empty;
  queue_type& _queue_full;
  FILE* _file;
  std::size_t _to_read;

public:
  reader ( queue_type& queue_empty
         , queue_type& queue_full
         , const std::string& filename
         , long begin
         , long end
         )
    : _queue_empty (queue_empty)
    , _queue_full (queue_full)
    , _file (fopen (filename.c_str(), "rb"))
    , _to_read ((end - begin)/sizeof(double))
  {
    if (!_file)
      {
        throw std::runtime_error ("could not open file " + filename);
      }

    fseek (_file, begin, SEEK_SET);
  }

  void operator () ()
  {
    buffer_type buffer;

    do
      {
        buffer = _queue_empty.get();

        buffer.count() = fread ( buffer.begin()
                               , sizeof (T)
                               , std::min (buffer.size(), _to_read)
                               , _file
                               );

        _to_read -= buffer.count();

        _queue_full.put (buffer);
      }
    while (buffer.count() != 0);

    fclose (_file);

    if (_to_read != 0)
      {
        throw std::runtime_error ("could not read all data");
      }
  }
};
