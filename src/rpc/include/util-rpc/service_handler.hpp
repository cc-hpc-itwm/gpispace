// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-rpc/detail/unique_scoped_handler_insert.hpp>
#include <util-rpc/function_description.hpp>
#include <util-rpc/service_dispatcher.hpp>

namespace fhg
{
  namespace rpc
  {
    constexpr struct{} const yielding{};
    constexpr struct{} const not_yielding{};

    //! A \c service_handler is the provider-side implementation of a
    //! function \a Description which has been declared using \c
    //! FHG_RPC_FUNCTION_DESCRIPTION.
    template<typename Description>
      struct service_handler
    {
      static_assert ( is_function_description_t<Description>::value
                    , "Description shall be a FHG_RPC_FUNCTION_DESCRIPTION"
                    );

    public:
      //! Register a handler for \c Description in \a dispatcher. When
      //! the dispatcher calls the handler, \a handler is invoked with
      //! the arguments as given in the \c remote_function call.
      //!
      //! \note Due to the way functions are dispatched, a \c
      //! service_handler is not allowed to be blocking with *any*
      //! other \c service_handler or operation on the same \c
      //! io_service (regardless of #threads and \c
      //! service_dispatcher).
      //!
      //! If the handler needs to be blocking, \c yielding shall be
      //! passed as third argument, which results in the arguments to
      //! \a handler being prefixed with a \c
      //! ::boost::asio::yield_context which can then be used to
      //! e.g. call \c remote_function from the handler itself,
      //! cooperatively.
      //!
      //! ```
      //!   service_handler<proto::fun> const fun_handler
      //!     (dispatcher, [] (int i) { return 2 * i; });
      //!
      //!   // This handler recursively calls `fon`, which is blocking,
      //!   // thus needs to be yielding:
      //!   service_handler<proto::fun> const fun_handler
      //!     ( dispatcher
      //!     , [&c] (::boost::asio::yield_context yield, int i)
      //!       {
      //!         return sync_remote_function<proto::fon> {c} (yield, i);
      //!       }
      //!     , yielding
      //!     );
      //! ```
      template<typename Func, typename Yielding = decltype (not_yielding)>
        service_handler ( service_dispatcher& dispatcher
                        , Func&& handler
                        , Yielding = Yielding{}
                        );

      service_handler (service_handler const&) = delete;
      service_handler (service_handler&&) = delete;
      auto operator= (service_handler const&) -> service_handler& = delete;
      auto operator= (service_handler&&) -> service_handler& = delete;

    private:
      detail::unique_scoped_handler_insert<decltype (service_dispatcher::_handlers)>
        _handler_registration;
    };
  }
}

#include <util-rpc/service_handler.ipp>
