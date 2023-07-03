// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <type_traits>
#include <utility>

namespace fhg
{
  namespace rpc
  {
    // \todo C++20: use concept instead of type trait using
    // `template<typename Protocol, SocketTraits<Protocol> Traits>`.
    //     template<typename Traits, typename ForProtocol>
    //       concept SocketTraits = requires (ForProtocol::socket& socket) {
    //         {Traits::apply_socket_options (socket)} -> std::same_as<void>;
    //     };

    //! `std::bool_constant` to determine whether \a SocketTraits are
    //! valid traits for a socket for the protocol \a ForProtocol.
    template<typename SocketTraits, typename ForProtocol>
      using is_socket_traits_t = std::is_void
        < decltype ( SocketTraits::apply_socket_options
                       (std::declval<typename ForProtocol::socket&>())
                   )
        >;
  }
}
