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

#include <boost/optional.hpp>

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      //! Insert a \c service_dispatcher handler into the given \a
      //! Handlers map for the lifetime of this object. The function
      //! name is ensured to not exist before insertion.
      //! \note A template to not make the \a Handlers type public.
      template<typename Handlers>
        struct unique_scoped_handler_insert
      {
        unique_scoped_handler_insert
          ( Handlers& handlers
          , typename Handlers::key_type name
          , typename Handlers::mapped_type function
          );
        unique_scoped_handler_insert
          (unique_scoped_handler_insert<Handlers>&& other);
        ~unique_scoped_handler_insert();

        unique_scoped_handler_insert
          (unique_scoped_handler_insert<Handlers> const&) = delete;
        unique_scoped_handler_insert<Handlers>& operator=
          (unique_scoped_handler_insert<Handlers> const&) = delete;
        unique_scoped_handler_insert<Handlers>& operator=
          (unique_scoped_handler_insert<Handlers>&&) = delete;

        Handlers& _handlers;
        boost::optional<typename Handlers::key_type> _name;
      };
    }
  }
}

#include <rpc/detail/unique_scoped_handler_insert.ipp>
