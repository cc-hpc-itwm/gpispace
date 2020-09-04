// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <rpc/common.hpp>
#include <rpc/function_description.hpp>
#include <rpc/service_dispatcher.hpp>

#include <boost/noncopyable.hpp>

namespace fhg
{
  namespace rpc
  {
    constexpr struct{} const yielding{};
    constexpr struct{} const not_yielding{};

    template<typename Description>
      struct service_handler : boost::noncopyable
    {
      static_assert ( is_function_description_t<Description>::value
                    , "Description shall be a FHG_RPC_FUNCTION_DESCRIPTION"
                    );

    public:
      //! Due to the way functions are dispatched, a service handler
      //! is not allowed to be blocking with *any* other service
      //! handler or operation on the same io_service (regardless of
      //! #threads and service_dispatcher). If you need to be
      //! blocking, use `yielding` and be cooperative!
      template<typename Func, typename Yielding = decltype (not_yielding)>
        service_handler ( service_dispatcher&
                        , Func&&
                        , Yielding = Yielding{}
                        );

    private:
      util::unique_scoped_map_insert<decltype (service_dispatcher::_handlers)>
        _handler_registration;
    };
  }
}

#include <rpc/service_handler.ipp>
