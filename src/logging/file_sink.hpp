#pragma once

#include <logging/message.hpp>
#include <logging/tcp_receiver.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <fstream>
#include <functional>
#include <ostream>

namespace fhg
{
  namespace logging
  {
    namespace error
    {
      struct unable_to_write_to_file : std::runtime_error
      {
        unable_to_write_to_file (boost::filesystem::path const&);
      };
    }

    class basic_file_sink
    {
    public:
      basic_file_sink ( boost::filesystem::path const&
                      , std::function<void (std::ostream&, message const&)>
                      , boost::optional<std::size_t> flush_interval
                      );

    protected:
      void dispatch_append (message const&);

    private:
      void append (message const&);
      void append_no_flush (message const&);
      void maybe_flush();

      std::ofstream _stream;
      std::function<void (std::ostream&, message const&)> _formatter;
      std::size_t _emit_counter;
      boost::optional<std::size_t> _flush_interval;
      void (basic_file_sink::* _append) (message const&);
    };

    template<typename Receiver>
      class file_sink final : private basic_file_sink
    {
    public:
      file_sink ( typename Receiver::endpoint_t const&
                , boost::filesystem::path const&
                , std::function<void (std::ostream&, message const&)>
                , boost::optional<std::size_t> flush_interval
                );

    private:
      Receiver _receiver;
    };

    using tcp_file_sink = file_sink<tcp_receiver>;
  }
}

#include <logging/file_sink.ipp>
