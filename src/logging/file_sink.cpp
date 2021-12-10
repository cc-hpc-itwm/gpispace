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
      , _emit_counter (0)
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
