// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <buffer.hpp>
#include <errno.h>
#include <queue.hpp>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <string.h>

template<typename T>
class writer
{
public:
  using buffer_type = fhg::buffer::type<T>;
  using queue_type = fhg::thread::queue<buffer_type>;

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
