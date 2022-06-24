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

#include <logging/message.hpp>
#include <logging/stream_receiver.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <fstream>
#include <functional>
#include <ostream>
#include <stdexcept>

namespace fhg
{
  namespace logging
  {
    namespace error
    {
      struct unable_to_write_to_file : std::runtime_error
      {
        unable_to_write_to_file (::boost::filesystem::path const&);
      };
    }

    class file_sink
    {
    public:
      file_sink ( endpoint const&
                , ::boost::filesystem::path const&
                , std::function<void (std::ostream&, message const&)>
                , ::boost::optional<std::size_t> flush_interval
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
      ::boost::optional<std::size_t> _flush_interval;
      void (file_sink::* _append) (message const&);

      stream_receiver _receiver;
    };
  }
}
