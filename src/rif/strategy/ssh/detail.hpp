// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <libssh2.h>

#include <stdexcept>
#include <string>
#include <utility>

namespace libssh2
{
  namespace detail
  {
    namespace wrap
    {
      std::string error_code_to_string (int code);
      void throw_last_error (LIBSSH2_SESSION* session);

      template<typename Fun, Fun fun, typename Ret = void>
        struct cant_fail
      {
        template<typename... Args>
          Ret operator() (Args&&... args) const
        {
          return Ret (fun (std::forward<Args> (args)...));
        }
      };

      template<typename Fun, Fun fun, typename Ret>
        struct negative_fails_via_return_value
      {
        template<typename... Args>
          Ret operator() (Args&&... args) const
        {
          int const rc (fun (std::forward<Args> (args)...));
          if (rc < 0)
          {
            throw std::runtime_error
              ("libssh2: " + error_code_to_string (rc));
          }
          return Ret (rc);
        }
      };
      template<typename Fun, Fun fun>
        struct negative_fails_via_return_value<Fun, fun, void>
      {
        template<typename... Args>
          void operator() (Args&&... args) const
        {
          int const rc (fun (std::forward<Args> (args)...));
          if (rc < 0)
          {
            throw std::runtime_error
              ("libssh2: " + error_code_to_string (rc));
          }
        }
      };

      template<typename Fun, Fun fun, typename Ret>
        struct negative_fails_via_session_state
      {
        template<typename... Args>
          Ret operator() (LIBSSH2_SESSION* session, Args&&... args) const
        {
          int const rc (fun (session, std::forward<Args> (args)...));
          if (rc < 0)
          {
            throw_last_error (session);
          }
          return Ret (rc);
        }
      };
      template<typename Fun, Fun fun>
        struct negative_fails_via_session_state<Fun, fun, void>
      {
        template<typename... Args>
          void operator() (LIBSSH2_SESSION* session, Args&&... args) const
        {
          int const rc (fun (session, std::forward<Args> (args)...));
          if (rc < 0)
          {
            throw_last_error (session);
          }
        }
      };

      template<typename Fun, Fun fun, typename Ret>
        struct NULL_fails_with_unknown_error
      {
        template<typename... Args>
          Ret* operator() (Args&&... args) const
        {
          Ret* const ret (fun (std::forward<Args> (args)...));
          if (!ret)
          {
            throw std::runtime_error ("libssh2: unknown error");
          }
          return ret;
        }
      };
      template<typename Fun, Fun fun, typename Ret>
        struct NULL_fails_via_session_state
      {
        template<typename... Args>
          Ret* operator() (LIBSSH2_SESSION* session, Args&&... args) const
        {
          Ret* const ret (fun (session, std::forward<Args> (args)...));
          if (!ret)
          {
            throw_last_error (session);
          }
          return ret;
        }
      };
    }
    namespace wrapped
    {
#define REF(_fun) decltype (&_fun), &_fun

      extern wrap::negative_fails_via_return_value <REF (libssh2_init), void> init;
      extern wrap::cant_fail<REF (libssh2_exit), void> exit;

      extern wrap::NULL_fails_with_unknown_error<REF (libssh2_session_init_ex), LIBSSH2_SESSION> session_init_ex;
      extern wrap::negative_fails_via_session_state<REF (libssh2_session_handshake), void> session_handshake;
      extern wrap::negative_fails_via_session_state<REF (libssh2_session_disconnect_ex), void> session_disconnect_ex;
      extern wrap::negative_fails_via_return_value<REF (libssh2_session_free), void> session_free;

      extern wrap::negative_fails_via_session_state<REF (libssh2_userauth_publickey_fromfile_ex), void> userauth_publickey_fromfile_ex;

      extern wrap::NULL_fails_via_session_state<REF (libssh2_channel_open_ex), LIBSSH2_CHANNEL> channel_open_ex;
      extern wrap::negative_fails_via_return_value<REF (libssh2_channel_close), void> channel_close;
      extern wrap::negative_fails_via_return_value<REF (libssh2_channel_free), void> channel_free;

      extern wrap::negative_fails_via_return_value<REF (libssh2_channel_process_startup), void> channel_process_startup;
      //! \note actually can fail but sentinel value is not distinguishable from normal values.
      extern wrap::cant_fail<REF (libssh2_channel_get_exit_status), int> channel_get_exit_status;
      extern wrap::negative_fails_via_return_value<REF (libssh2_channel_get_exit_signal), void> channel_get_exit_signal;
      extern wrap::negative_fails_via_return_value<REF (libssh2_channel_read_ex), std::size_t> channel_read_ex;

#undef REF
    }
  }
}
