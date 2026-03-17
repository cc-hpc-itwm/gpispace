// Copyright (C) 2018-2019,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/logging/message.hpp>
#include <gspc/logging/stream_receiver.hpp>

#include <filesystem>
#include <fstream>
#include <functional>
#include <optional>
#include <ostream>
#include <stdexcept>


  namespace gspc::logging
  {
    namespace error
    {
      struct unable_to_write_to_file : std::runtime_error
      {
        unable_to_write_to_file (std::filesystem::path const&);
      };
    }

    class file_sink
    {
    public:
      file_sink ( endpoint const&
                , std::filesystem::path const&
                , std::function<void (std::ostream&, message const&)>
                , std::optional<std::size_t> flush_interval
                );

    protected:
      void dispatch_append (message const&);

    private:
      void append (message const&);
      void append_no_flush (message const&);
      void maybe_flush();

      std::ofstream _stream;
      std::function<void (std::ostream&, message const&)> _formatter;
      std::size_t _emit_counter {0};
      std::optional<std::size_t> _flush_interval;
      void (file_sink::* _append) (message const&);

      stream_receiver _receiver;
    };
  }
