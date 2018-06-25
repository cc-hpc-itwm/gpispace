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
          (boost::filesystem::path const& path)
        : std::runtime_error
            ("unable to open '" + path.string() + "' for writing")
      {}
    }

    basic_file_sink::basic_file_sink
        ( boost::filesystem::path const& target
        , std::function<void (std::ostream&, message const&)> formatter
        , boost::optional<std::size_t> flush_interval
        )
      : _stream()
      , _formatter (std::move (formatter))
      , _emit_counter (0)
      , _flush_interval (std::move (flush_interval))
      , _append ( !!_flush_interval
                ? &basic_file_sink::append
                : &basic_file_sink::append_no_flush
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

    void basic_file_sink::dispatch_append (message const& m)
    {
      return (this->*_append) (m);
    }
    void basic_file_sink::append (message const& message)
    {
      append_no_flush (message);
      maybe_flush();
    }
    void basic_file_sink::append_no_flush (message const& message)
    {
      _formatter (_stream, message);
    }

    void basic_file_sink::maybe_flush()
    {
      if (++_emit_counter >= _flush_interval.get())
      {
        _stream.flush();
        _emit_counter = 0;
      }
    }
  }
}
