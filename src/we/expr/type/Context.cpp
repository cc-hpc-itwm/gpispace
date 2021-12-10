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

#include <we/expr/type/Context.hpp>

#include <we/expr/type/AssignResult.hpp>
#include <we/expr/exception.hpp>

#include <util-generic/functor_visitor.hpp>

#include <boost/optional.hpp>
#include <boost/format.hpp>

#include <algorithm>
#include <exception>
#include <functional>
#include <iterator>
#include <stdexcept>

namespace expr
{
  namespace type
  {
    namespace
    {
      template<typename S, typename T>
        ::boost::optional<std::reference_wrapper<S>> as_struct (T& node) noexcept
      {
        using Ref = std::reference_wrapper<S>;

        return fhg::util::visit<::boost::optional<Ref>>
          ( node
          , [] (S& s) -> ::boost::optional<Ref>
            {
              return Ref {s};
            }
          , [&] (auto&&) -> ::boost::optional<Ref>
            {
              return {};
            }
          );
      }

      template<typename T, typename Fields>
        ::boost::optional<std::reference_wrapper<T>> find
          ( Path::const_iterator key
          , Fields& fields
          ) noexcept
      {
        auto const field
          ( std::find_if
            ( std::begin (fields), std::end (fields)
            , [&] (auto const& f)
              {
                return f._name == *key;
              }
            )
          );

        if (field != std::end (fields))
        {
          return std::reference_wrapper<T> {field->_type};
        }

        return {};
      }

      struct Bind
      {
        Bind ( Path const& path
             , Type const& type
             )
          : _path (path)
          , _type (type)
        {}

        Type chain (Path::const_iterator key) const
        {
          if (key == std::end (_path))
          {
            return _type;
          }

          return Struct ({{*key, chain (std::next (key))}});
        }

        Type _bind (Path::const_iterator key, Type& node)
        {
          if (key == std::end (_path))
          {
            return node = assign_result (_path, node, _type);
          }

          if (auto&& _struct = as_struct<Struct> (node))
          {
            auto& fields (_struct->get()._fields);

            if (auto field = find<Type> (key, fields))
            {
              return _bind (std::next (key), field->get());
            }
            else
            {
              fields.emplace ( std::end (fields)
                             , *key
                             , chain (std::next (key))
                             );

              return _type;
            }
          }

          throw exception::type::error
            ( ::boost::format ("Not a struct at %1%: '%2%'")
            % Path (std::begin (_path), key)
            % node
            );
        }

        Type operator() (Type& node)
        {
          return _bind (std::begin (_path), node);
        }

      private:
        Path const& _path;
        Type const& _type;
      };

      struct At
      {
        At (Path const& path) noexcept
          : _path (path)
        {}

        ::boost::optional<Type> _at
          ( Path::const_iterator key
          , Type const& node
          ) noexcept
        {
          if (key == std::end (_path))
          {
            return node;
          }

          if (node == Any())
          {
            return node;
          }

          if (auto&& _struct = as_struct<Struct const> (node))
          {
            auto const& fields (_struct->get()._fields);

            if (auto field = find<Type const> (key, fields))
            {
              return _at (std::next (key), field->get());
            }
          }

          return {};
        }

        ::boost::optional<Type> operator() (Type const& node) noexcept
        {
          return _at (std::begin (_path), node);
        }

      private:
        Path const& _path;
      };
    }

    Type Context::bind (Path const& path, Type const& type)
    try
    {
      return Bind {path, type} (_root);
    }
    catch (...)
    {
      std::throw_with_nested
        ( exception::type::error
          ( ::boost::format ("expr::type::Context::bind (%1%, '%2%')")
          % path
          % type
          )
        );
    }

    ::boost::optional<Type> Context::at (Path const& path) const noexcept
    {
      return At {path} (_root);
    }
  }
}
