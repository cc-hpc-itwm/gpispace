// Copyright (C) 2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <boost/program_options/variables_map.hpp>


  namespace gspc::iml::detail
  {
    // \todo Use the "generic" infrastructure?

    template<typename Type, typename As>
      std::optional<Type> get
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


#include <gspc/iml/detail/option.ipp>
