// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/rif/strategy/ssh/context.hpp>

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
