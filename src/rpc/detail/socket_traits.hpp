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
