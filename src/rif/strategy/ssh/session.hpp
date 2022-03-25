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

#include <rif/strategy/ssh/context.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/variant.hpp>

#include <string>
#include <tuple>
#include <utility>

struct _LIBSSH2_SESSION;

namespace libssh2
{
  struct session
  {
    //! \note Assumes socket_fd is BLOCKING. Will horribly break otherwise.
    session ( context&
            , int socket_fd
            , std::string const& username
            , std::pair<::boost::filesystem::path, ::boost::filesystem::path>
                const& public_and_private_key
            );

    ~session();
    session (session const&) = delete;
    session (session&&) = delete;
    session& operator= (session const&) = delete;
    session& operator= (session&&) = delete;

    struct execute_return_type
    {
      enum
      {
        signaled,
        exited,
      } method_of_returning;

      int return_value;
      std::string exit_signal;

      std::string stdout;
      std::string stderr;
    };
    execute_return_type execute (std::string const&);
    execute_return_type execute_and_require_success_and_no_output
      (std::string const&, bool no_stdout, bool no_stderr);
    void execute_and_require_success_and_no_output (std::string const&);
    std::string execute_and_require_success_and_no_stderr_output (std::string const&);

  private:
    int _socket_fd;
    _LIBSSH2_SESSION* _;
  };
}
