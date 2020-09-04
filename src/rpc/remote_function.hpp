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
#include <rpc/future.hpp>
#include <rpc/remote_endpoint.hpp>

#include <future>

namespace fhg
{
  namespace rpc
  {
    //! \note While exceptions will be transferred from remote, note
    //! that it requires custom exception types to be registered in
    //! the remote_endpoint and service_dispatcher. Else, exceptions
    //! will be down-cast to the nearest std::* exception class; or
    //! service_dispatcher will std::terminate() when the exception
    //! does not inherit from a std::* exception class. Note that if
    //! an exception serialization is registered in
    //! service_dispatcher, it needs to be registered in
    //! remote_endpoint as well to properly deserialize and avoid an
    //! std::terminate() it does not inherit from a std::* exception
    //! class. If it does, it will be downcasted automatically. On the
    //! API to implement, see src/rpc/exception_serialization.hpp.

    template < typename Description
             , template<typename> class Future = std::future
             >
      struct remote_function
    {
    private:
      static_assert ( is_function_description_t<Description>::value
                    , "Description shall be a FHG_RPC_FUNCTION_DESCRIPTION"
                    );

    public:
      remote_function (remote_endpoint&);

      template<typename... Args>
        Future<typename Description::result_type> operator() (Args&&...);

    private:
      remote_endpoint& _endpoint;
    };

    template < typename Description
             , template<typename> class Future = std::future
             >
      struct sync_remote_function
    {
    private:
      static_assert ( is_function_description_t<Description>::value
                    , "Description shall be a FHG_RPC_FUNCTION_DESCRIPTION"
                    );

    public:
      sync_remote_function (remote_endpoint&);

      //! \note When using a \c rpc::future, pass the yield context as
      //! first argument.
      template<typename... Args>
        typename Description::result_type operator() (Args&&...);

    private:
      remote_function<Description, Future> _function;
    };
  }
}

#include <rpc/remote_function.ipp>
