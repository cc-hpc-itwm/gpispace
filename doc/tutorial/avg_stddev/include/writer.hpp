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