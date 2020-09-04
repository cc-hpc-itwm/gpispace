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

#include <boost/iterator/transform_iterator.hpp>
#include <boost/optional.hpp>

#include <unordered_map>

namespace fhg
{
  namespace pnet
  {
    namespace util
    {
      template<typename T>
        struct unique
      {
        using key_type = typename T::unique_key_type;

      private:
        std::unordered_map<key_type, T> _values;

        static T const& select (typename decltype (_values)::value_type const& v)
        {
          return v.second;
        }
        static T& select_mutable (typename decltype (_values)::value_type& v)
        {
          return v.second;
        }

      public:
        using value_type = T;
        using iterator = boost::transform_iterator
          <decltype (&select_mutable), typename decltype (_values)::iterator>;
        using const_iterator = boost::transform_iterator
          <decltype (&select), typename decltype (_values)::const_iterator>;

        template<typename DuplicateException>
          void push (value_type const& value)
        {
          auto const result (_values.emplace (value.unique_key(), value));
          if (!result.second)
          {
            throw DuplicateException (result.first->second, value);
          }
        }

        boost::optional<T const&> get (key_type const& key) const
        {
          auto const it (_values.find (key));
          if (it == _values.end())
          {
            return boost::none;
          }
          return it->second;
        }

        bool has (key_type const& key) const
        {
          return _values.count (key);
        }

        auto size() const -> decltype (_values.size()) { return _values.size(); }
        auto empty() const -> decltype (_values.empty()) { return _values.empty(); }

        const_iterator begin() const { return {_values.begin(), &select}; }
        const_iterator end() const { return {_values.end(), &select}; }
        iterator begin() { return {_values.begin(), &select_mutable}; }
        iterator end() { return {_values.end(), &select_mutable}; }
      };
    }
  }
}
