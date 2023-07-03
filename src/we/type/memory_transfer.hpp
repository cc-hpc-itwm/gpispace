// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/expr/type/Context.hpp>

#include <boost/optional/optional.hpp>
#include <boost/serialization/access.hpp>

#include <string>

namespace we
{
  namespace type
  {
    struct memory_transfer
    {
    public:
      //! \note serialization only
      memory_transfer();

      memory_transfer
        ( std::string const& global
        , std::string const& local
        , ::boost::optional<bool> const& not_modified_in_module_call
        , bool allow_empty_ranges
        );
      std::string const& global() const;
      std::string const& local() const;
      ::boost::optional<bool> const& not_modified_in_module_call() const;
      bool const& allow_empty_ranges() const;

      void assert_correct_expression_types
        (expr::type::Context const&) const;

    private:
      std::string _global;
      std::string _local;
      ::boost::optional<bool> _not_modified_in_module_call;
      bool _allow_empty_ranges;

      friend class ::boost::serialization::access;
      template<class Archive> void serialize (Archive&, unsigned int);
    };
  }
}

#include <we/type/memory_transfer.ipp>
