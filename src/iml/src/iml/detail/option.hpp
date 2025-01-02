// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/optional.hpp>
#include <boost/program_options/variables_map.hpp>

namespace iml
{
  namespace detail
  {
    // \todo Use the "generic" infrastructure?

    template<typename Type, typename As>
      ::boost::optional<Type> get
        (::boost::program_options::variables_map const& vm, char const* name);

    template<typename Type, typename As>
      Type require
        (::boost::program_options::variables_map const& vm, char const* name);

    template<typename Type, typename As, typename ToString>
      void set ( ::boost::program_options::variables_map& vm
               , char const* name
               , As value
               , ToString&& to_string
               );
  }
}

#include <iml/detail/option.ipp>
