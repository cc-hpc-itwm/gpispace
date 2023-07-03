// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
