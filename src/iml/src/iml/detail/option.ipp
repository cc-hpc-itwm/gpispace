// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fmt/core.h>
#include <stdexcept>
#include <string>

namespace iml
{
  namespace detail
  {
    template<typename Type, typename As>
      ::boost::optional<Type> get
        (::boost::program_options::variables_map const& vm, char const* name)
    try
    {
      if (vm.count (name))
      {
        return Type {vm.at (name).as<As>()};
      }

      return ::boost::none;
    }
    catch (...)
    {
      std::throw_with_nested
        ( std::runtime_error
            ("When retrieving command line option --" + std::string (name))
        );
    }

    template<typename Type, typename As>
      Type require
        (::boost::program_options::variables_map const& vm, char const* name)
    {
      auto result (get<Type, As> (vm, name));
      if (result)
      {
        return *result;
      }

      throw std::logic_error
        {fmt::format ("missing key '{}' in variables map", name)};
    }

    template<typename Type, typename As, typename ToString>
      void set ( ::boost::program_options::variables_map& vm
               , char const* name
               , As value
               , ToString&& to_string
               )
    {
      auto const pos_and_success
        ( vm.emplace
            (name, ::boost::program_options::variable_value (value, false))
        );

      if (!pos_and_success.second)
      {
        throw std::runtime_error
          { fmt::format
              ( "Failed to set option '{}' to '{}': Found old value '{}'"
              , name
              , to_string (Type (value))
              , to_string (Type (pos_and_success.first->second.as<As>()))
              )
          };
      }
    }
  }
}
