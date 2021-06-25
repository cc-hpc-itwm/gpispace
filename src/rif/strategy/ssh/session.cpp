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

#include <rif/strategy/ssh/session.hpp>

#include <rif/strategy/ssh/detail.hpp>

#include <fhg/util/next.hpp>

#include <util-generic/nest_exceptions.hpp>

struct _LIBSSH2_CHANNEL;

namespace libssh2
{
  session::session ( context&
                   , int socket_fd
                   , std::string const& username
                   , std::pair<boost::filesystem::path, boost::filesystem::path>
                       const& public_and_private_key
                   )
  try
    : _socket_fd (socket_fd)
    , _ (detail::wrapped::session_init_ex (nullptr, nullptr, nullptr, nullptr))
  {
    fhg::util::nest_exceptions<std::runtime_error>
      ( [&]
        {
          detail::wrapped::session_handshake (_, _socket_fd);
        }
      , "handshake failed"
      );
    fhg::util::nest_exceptions<std::runtime_error>
      ( [&]
        {
          detail::wrapped::userauth_publickey_fromfile_ex
            ( _
            , username.c_str()
            , username.size()
            , public_and_private_key.first.string().c_str()
            , public_and_private_key.second.string().c_str()
            , nullptr
            );
        }
      , "authentication failed"
      );
  }
  catch (...)
  {
    std::throw_with_nested
      (std::runtime_error ("initializing ssh session failed"));
  }

  session::~session()
  {
    detail::wrapped::session_disconnect_ex
      (_, SSH_DISCONNECT_BY_APPLICATION, nullptr, nullptr);
    detail::wrapped::session_free (_);
  }

  namespace
  {
    struct channel_wrapper
    {
      channel_wrapper (LIBSSH2_SESSION* session)
      try
        : _ ( detail::wrapped::channel_open_ex
                ( session
                , "session"
                , strlen ("session")
                , LIBSSH2_CHANNEL_WINDOW_DEFAULT
                , LIBSSH2_CHANNEL_PACKET_DEFAULT
                , nullptr
                , 0u
                )
            )
      {}
      catch (...)
      {
        std::throw_with_nested
          (std::runtime_error ("creating communication channel failed"));
      }

      ~channel_wrapper()
      {
        detail::wrapped::channel_close (_);
        detail::wrapped::channel_free (_);
      }

      _LIBSSH2_CHANNEL* _;
    };

    std::string read_all_from_stream (LIBSSH2_CHANNEL* channel, int stream)
    {
      std::vector<char> buffer (0x1000);
      std::size_t bytes_read_total (0);
      std::size_t bytes_read_or_rc (0);

      do
      {
        bytes_read_or_rc
          = detail::wrapped::channel_read_ex
              ( channel
              , stream
              , buffer.data() + bytes_read_total
              , buffer.size() - bytes_read_total
              );

        if (buffer.size() - bytes_read_total == bytes_read_or_rc)
        {
          buffer.resize (buffer.size() * 2);
        }

        bytes_read_total += bytes_read_or_rc;
      }
      while (bytes_read_or_rc);

      return {buffer.begin(), fhg::util::next (buffer.begin(), bytes_read_total)};
    }

    session::execute_return_type execute_impl ( LIBSSH2_SESSION* session
                                              , std::string const& command
                                              )
    {
      channel_wrapper const channel (session);

      detail::wrapped::channel_process_startup
        (channel._, "exec", strlen ("exec"), command.c_str(), command.size());

      std::string const stdout
        (read_all_from_stream (channel._, 0));
      std::string const stderr
        (read_all_from_stream (channel._, SSH_EXTENDED_DATA_STDERR));

      char* exitsignal (nullptr);
      detail::wrapped::channel_get_exit_signal
        (channel._, &exitsignal, nullptr, nullptr, nullptr, nullptr, nullptr);

      if (exitsignal)
      {
        std::string const signal (exitsignal);
        //! \note HACK! assumes libssh2_default_free to be
        //! free(). would be _session._->free (exitsignal,
        //! _session._->abstract), but _session._ is only defined in
        //! libssh2_priv.h. There does not appear to be a way to get
        //! this function.
        free (exitsignal);

        return {session::execute_return_type::signaled, 0, signal, stdout, stderr};
      }

      int const exitcode
        (detail::wrapped::channel_get_exit_status (channel._));

      return {session::execute_return_type::exited, exitcode, "", stdout, stderr};
    }
  }

  session::execute_return_type session::execute (std::string const& command)
  try
  {
    return execute_impl (_, command);
  }
  catch (...)
  {
    std::throw_with_nested
      (std::runtime_error ("executing '" + command + "' failed"));
  }

  session::execute_return_type session::execute_and_require_success_and_no_output
    (std::string const& command, bool no_stdout, bool no_stderr)
  {
    execute_return_type result (execute_impl (_, command));

    try
    {
      if (result.method_of_returning == execute_return_type::signaled)
      {
        throw std::runtime_error ("signaled " + result.exit_signal);
      }
      else if (result.return_value)
      {
        throw std::runtime_error
          ("returned " + std::to_string (result.return_value));

      }
      else if (no_stdout && !result.stdout.empty())
      {
        throw std::runtime_error ("output not empty");
      }
      else if (no_stderr && !result.stderr.empty())
      {
        throw std::runtime_error ("error output not empty");
      }
    }
    catch (...)
    {
      std::throw_with_nested ( std::runtime_error
                                 ( "executing '" + command + "' failed, "
                                 + " out: '" + result.stdout
                                 + "' err: '" + result.stderr + "'"
                                 )
                             );
    }

    return result;
  }

  void session::execute_and_require_success_and_no_output
    (std::string const& command)
  {
    execute_and_require_success_and_no_output (command, true, true);
  }

  std::string session::execute_and_require_success_and_no_stderr_output
    (std::string const& command)
  {
    return execute_and_require_success_and_no_output
      (command, false, true).stdout;
  }
}
