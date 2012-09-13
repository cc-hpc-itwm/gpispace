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
