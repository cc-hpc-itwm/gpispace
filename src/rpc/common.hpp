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

#include <util-generic/hash/boost/asio/ip/tcp/endpoint.hpp>
#include <util-generic/serialization/boost/asio/ip/tcp/endpoint.hpp>
#include <util-generic/serialization/exception.hpp>

#include <boost/iostreams/concepts.hpp>
#include <boost/optional.hpp>

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace fhg
{
  namespace rpc
  {
    struct packet_header
    {
      uint64_t message_id;
      uint64_t buffer_size;

      packet_header() = default;
      packet_header (uint64_t id, uint64_t size)
        : message_id (id)
        , buffer_size (size)
      {}
    };

    namespace error
    {
      struct duplicate_function : std::logic_error
      {
        std::string function_name;
        duplicate_function (decltype (function_name) name)
          : std::logic_error ("duplicate function name '" + name + "'")
          , function_name (std::move (name))
        {}

        std::string to_string() const { return function_name; }
        static duplicate_function from_string (std::string name) { return name; }
      };

      struct unknown_function : std::logic_error
      {
        std::string function_name;
        unknown_function (decltype (function_name) name)
          : std::logic_error  ("function '" + name + "' does not exist")
          , function_name (std::move (name))
        {}

        std::string to_string() const { return function_name; }
        static unknown_function from_string (std::string name) { return name; }
      };

      using namespace util::serialization;
      inline exception::serialization_functions add_builtin
        (exception::serialization_functions functions)
      {
        functions.emplace (exception::make_functions<duplicate_function>());
        functions.emplace (exception::make_functions<unknown_function>());
        return functions;
      }
    }
  }

  namespace util
  {
    template<typename Container>
      struct unique_scoped_map_insert
    {
      unique_scoped_map_insert ( Container& container
                               , typename Container::key_type key
                               , typename Container::mapped_type value
                               )
        : _container (container)
        , _key (boost::none)
      {
        if (!_container.emplace (key, std::move (value)).second)
        {
          throw rpc::error::duplicate_function (std::move (key));
        }

        _key = std::move (key);
      }
      unique_scoped_map_insert (unique_scoped_map_insert<Container>&& other)
        : _container (std::move (other._container))
        , _key (boost::none)
      {
        std::swap (_key, other._key);
      }
      ~unique_scoped_map_insert()
      {
        if (_key)
        {
          _container.erase (*_key);
        }
      }

      unique_scoped_map_insert (unique_scoped_map_insert<Container> const&)
        = delete;
      unique_scoped_map_insert<Container>& operator=
        (unique_scoped_map_insert<Container> const&) = delete;
      unique_scoped_map_insert<Container>& operator=
        (unique_scoped_map_insert<Container>&&) = delete;

      Container& _container;
      boost::optional<typename Container::key_type> _key;
    };

    struct vector_sink : boost::iostreams::sink
    {
      vector_sink (std::vector<char>& vector) : _vector (vector) {}

      std::streamsize write (char_type const* s, std::streamsize n)
      {
        _vector.insert (_vector.end(), s, s + n);
        return n;
      }

    private:
      std::vector<char>& _vector;
    };
  }
}
