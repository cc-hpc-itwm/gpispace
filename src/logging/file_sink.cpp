// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <logging/file_sink.hpp>

#include <util-generic/this_bound_mem_fn.hpp>

#include <exception>

namespace fhg
{
  namespace logging
  {
    namespace error
    {
      unable_to_write_to_file::unable_to_write_to_file
          (::boost::filesystem::path const& path)
        : std::runtime_error
            ("unable to open '" + path.string() + "' for writing")
      {}
    }

    file_sink::file_sink
        ( endpoint const& emitter
        , ::boost::filesystem::path const& target
        , std::function<void (std::ostream&, message const&)> formatter
        , ::boost::optional<std::size_t> flush_interval
        )
      : _stream()
      , _formatter (std::move (formatter))
      , _flush_interval (std::move (flush_interval))
      , _append ( !!_flush_interval
                ? &file_sink::append
                : &file_sink::append_no_flush
                )
      , _receiver
          ( emitter
          , [&] (message const& m) { return dispatch_append (m); }
          )
    {
      _stream.exceptions (std::fstream::badbit | std::fstream::failbit);
      try
      {
        _stream.open
          (target.string(), std::fstream::app | std::fstream::binary);
      }
      catch (...)
      {
        std::throw_with_nested (error::unable_to_write_to_file (target));
      }
    }

    void file_sink::dispatch_append (message const& m)
    {
      return (this->*_append) (m);
    }
    void file_sink::append (message const& message)
    {
      append_no_flush (message);
      maybe_flush();
    }
    void file_sink::append_no_flush (message const& message)
    {
      _formatter (_stream, message);
    }

    void file_sink::maybe_flush()
    {
      if (++_emit_counter >= _flush_interval.get())
      {
        _stream.flush();
        _emit_counter = 0;
      }
    }
  }
}
