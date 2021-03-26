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

#include <rif/strategy/ssh/detail.hpp>

namespace libssh2
{
  namespace detail
  {
    namespace wrap
    {
      std::string error_code_to_string (int code)
      {
        switch (code)
        {
        case LIBSSH2_ERROR_NONE: return "NONE";
        case LIBSSH2_ERROR_SOCKET_NONE: return "SOCKET_NONE";
        case LIBSSH2_ERROR_BANNER_RECV: return "BANNER_RECV";
        case LIBSSH2_ERROR_BANNER_SEND: return "BANNER_SEND";
        case LIBSSH2_ERROR_INVALID_MAC: return "INVALID_MAC";
        case LIBSSH2_ERROR_KEX_FAILURE: return "KEX_FAILURE";
        case LIBSSH2_ERROR_ALLOC: return "ALLOC";
        case LIBSSH2_ERROR_SOCKET_SEND: return "SOCKET_SEND";
        case LIBSSH2_ERROR_KEY_EXCHANGE_FAILURE: return "KEY_EXCHANGE_FAILURE";
        case LIBSSH2_ERROR_TIMEOUT: return "TIMEOUT";
        case LIBSSH2_ERROR_HOSTKEY_INIT: return "HOSTKEY_INIT";
        case LIBSSH2_ERROR_HOSTKEY_SIGN: return "HOSTKEY_SIGN";
        case LIBSSH2_ERROR_DECRYPT: return "DECRYPT";
        case LIBSSH2_ERROR_SOCKET_DISCONNECT: return "SOCKET_DISCONNECT";
        case LIBSSH2_ERROR_PROTO: return "PROTO";
        case LIBSSH2_ERROR_PASSWORD_EXPIRED: return "PASSWORD_EXPIRED";
        case LIBSSH2_ERROR_FILE: return "FILE";
        case LIBSSH2_ERROR_METHOD_NONE: return "METHOD_NONE";
        case LIBSSH2_ERROR_AUTHENTICATION_FAILED: return "AUTHENTICATION_FAILED";
        case LIBSSH2_ERROR_PUBLICKEY_UNVERIFIED: return "PUBLICKEY_UNVERIFIED";
        case LIBSSH2_ERROR_CHANNEL_OUTOFORDER: return "CHANNEL_OUTOFORDER";
        case LIBSSH2_ERROR_CHANNEL_FAILURE: return "CHANNEL_FAILURE";
        case LIBSSH2_ERROR_CHANNEL_REQUEST_DENIED: return "CHANNEL_REQUEST_DENIED";
        case LIBSSH2_ERROR_CHANNEL_UNKNOWN: return "CHANNEL_UNKNOWN";
        case LIBSSH2_ERROR_CHANNEL_WINDOW_EXCEEDED: return "CHANNEL_WINDOW_EXCEEDED";
        case LIBSSH2_ERROR_CHANNEL_PACKET_EXCEEDED: return "CHANNEL_PACKET_EXCEEDED";
        case LIBSSH2_ERROR_CHANNEL_CLOSED: return "CHANNEL_CLOSED";
        case LIBSSH2_ERROR_CHANNEL_EOF_SENT: return "CHANNEL_EOF_SENT";
        case LIBSSH2_ERROR_SCP_PROTOCOL: return "SCP_PROTOCOL";
        case LIBSSH2_ERROR_ZLIB: return "ZLIB";
        case LIBSSH2_ERROR_SOCKET_TIMEOUT: return "SOCKET_TIMEOUT";
        case LIBSSH2_ERROR_SFTP_PROTOCOL: return "SFTP_PROTOCOL";
        case LIBSSH2_ERROR_REQUEST_DENIED: return "REQUEST_DENIED";
        case LIBSSH2_ERROR_METHOD_NOT_SUPPORTED: return "METHOD_NOT_SUPPORTED";
        case LIBSSH2_ERROR_INVAL: return "INVAL";
        case LIBSSH2_ERROR_INVALID_POLL_TYPE: return "INVALID_POLL_TYPE";
        case LIBSSH2_ERROR_PUBLICKEY_PROTOCOL: return "PUBLICKEY_PROTOCOL";
        case LIBSSH2_ERROR_EAGAIN: return "EAGAIN";
        case LIBSSH2_ERROR_BUFFER_TOO_SMALL: return "BUFFER_TOO_SMALL";
        case LIBSSH2_ERROR_BAD_USE: return "BAD_USE";
        case LIBSSH2_ERROR_COMPRESS: return "COMPRESS";
        case LIBSSH2_ERROR_OUT_OF_BOUNDARY: return "OUT_OF_BOUNDARY";
        case LIBSSH2_ERROR_AGENT_PROTOCOL: return "AGENT_PROTOCOL";
        case LIBSSH2_ERROR_SOCKET_RECV: return "SOCKET_RECV";
        case LIBSSH2_ERROR_ENCRYPT: return "ENCRYPT";
        case LIBSSH2_ERROR_BAD_SOCKET: return "BAD_SOCKET";
        case LIBSSH2_ERROR_KNOWN_HOSTS: return "KNOWN_HOSTS";
        }
        return "UNKNOWN";
      }

      void throw_last_error (LIBSSH2_SESSION* session)
      {
        char* message (nullptr);
        int const code ( libssh2_session_last_error
                           (session, &message, nullptr, false)
                       );
        if (code)
        {
          throw std::runtime_error
            ( "libssh2: " + error_code_to_string (code) + ": "
            + std::string (message)
            );
        }
      }
    }
    namespace wrapped
    {
#define REF(_fun) decltype (&_fun), &_fun

      wrap::negative_fails_via_return_value <REF (libssh2_init), void> init;
      wrap::cant_fail<REF (libssh2_exit), void> exit;

      wrap::NULL_fails_with_unknown_error<REF (libssh2_session_init_ex), LIBSSH2_SESSION> session_init_ex;
      wrap::cant_fail<REF (libssh2_session_set_blocking), void> session_set_blocking;
      wrap::negative_fails_via_session_state<REF (libssh2_session_handshake), void> session_handshake;
      wrap::negative_fails_via_session_state<REF (libssh2_session_disconnect_ex), void> session_disconnect_ex;
      wrap::negative_fails_via_return_value<REF (libssh2_session_free), void> session_free;

      wrap::negative_fails_via_session_state<REF (libssh2_userauth_publickey_fromfile_ex), void> userauth_publickey_fromfile_ex;

      wrap::NULL_fails_via_session_state<REF (libssh2_channel_open_ex), LIBSSH2_CHANNEL> channel_open_ex;
      wrap::negative_fails_via_return_value<REF (libssh2_channel_close), void> channel_close;
      wrap::negative_fails_via_return_value<REF (libssh2_channel_free), void> channel_free;

      wrap::negative_fails_via_return_value<REF (libssh2_channel_process_startup), void> channel_process_startup;
      //! \note actually can fail but sentinel value is not distinguishable from normal values.
      wrap::cant_fail<REF (libssh2_channel_get_exit_status), int> channel_get_exit_status;
      wrap::negative_fails_via_return_value<REF (libssh2_channel_get_exit_signal), void> channel_get_exit_signal;
      wrap::negative_fails_via_return_value<REF (libssh2_channel_read_ex), std::size_t> channel_read_ex;

#undef REF
    }
  }
}
