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

#include <exception>
#include <future>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <vector>

namespace fhg
{
  namespace util
  {
    void throw_collected_exceptions (std::vector<std::exception_ptr> const&);

    template<typename Container, typename Show>
      void throw_collected_exceptions (Container const& container, Show&& show)
    {
      std::vector<std::exception_ptr> exceptions;

      for (typename Container::value_type const& element : container)
      {
        exceptions.emplace_back
          (std::make_exception_ptr (std::runtime_error (show (element))));
      }

      throw_collected_exceptions (exceptions);
    }

    template<typename Collection, typename Operation>
      void apply_for_each_and_collect_exceptions
        (Collection&& collection, Operation&& operation)
    {
      std::vector<std::exception_ptr> exceptions;

      for (auto& element : collection)
      {
        try
        {
          operation (element);
        }
        catch (...)
        {
          exceptions.push_back (std::current_exception());
        }
      }

      throw_collected_exceptions (exceptions);
    }

    //! \note Helper for overload resolution
    template<typename Element, typename Operation>
      void apply_for_each_and_collect_exceptions
        (std::initializer_list<Element>&& collection, Operation&& operation)
    {
      apply_for_each_and_collect_exceptions<std::initializer_list<Element>>
        (std::move (collection), std::forward<Operation> (operation));
    }

    void wait_and_collect_exceptions (std::vector<std::future<void>>&);
  }
}
