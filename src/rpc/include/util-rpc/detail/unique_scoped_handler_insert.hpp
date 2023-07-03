// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
          (unique_scoped_handler_insert<Handlers>&& other) noexcept;
        ~unique_scoped_handler_insert();

        unique_scoped_handler_insert
          (unique_scoped_handler_insert<Handlers> const&) = delete;
        unique_scoped_handler_insert<Handlers>& operator=
          (unique_scoped_handler_insert<Handlers> const&) = delete;
        unique_scoped_handler_insert<Handlers>& operator=
          (unique_scoped_handler_insert<Handlers>&&) = delete;

        Handlers& _handlers;
        ::boost::optional<typename Handlers::key_type> _name;
      };
    }
  }
}

#include <util-rpc/detail/unique_scoped_handler_insert.ipp>
