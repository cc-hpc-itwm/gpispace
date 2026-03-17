// Copyright (C) 2021-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/expr/type/Context.hpp>

#include <gspc/we/expr/exception.hpp>
#include <gspc/we/expr/type/AssignResult.hpp>

#include <gspc/util/functor_visitor.hpp>

#include <optional>

#include <gspc/util/fmt/boost/variant.formatter.hpp>
#include <gspc/we/expr/type/Path.formatter.hpp>
#include <gspc/we/expr/type/Type.formatter.hpp>
#include <algorithm>
#include <exception>
#include <fmt/core.h>
#include <functional>
#include <iterator>
#include <stdexcept>


  namespace gspc::we::expr::type
  {
    namespace
    {
      template<typename S, typename T>
        std::optional<std::reference_wrapper<S>> as_struct (T& node) noexcept
      {
        using Ref = std::reference_wrapper<S>;

        return util::visit<std::optional<Ref>>
          ( node
          , [] (S& s) -> std::optional<Ref>
            {
              return Ref {s};
            }
          , [&] (auto&&) -> std::optional<Ref>
            {
              return {};
            }
          );
      }

      template<typename T, typename Fields>
        std::optional<std::reference_wrapper<T>> find
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
            { fmt::format ( "Not a struct at {}: '{}'"
                          , Path (std::begin (_path), key)
                          , node
                          )
            };
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

        std::optional<Type> _at
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

        std::optional<Type> operator() (Type const& node) noexcept
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
          { fmt::format ( "gspc::we::expr::type::Context::bind ({}, '{}')"
                        , path
                        , type
                        )
          }
        );
    }

    std::optional<Type> Context::at (Path const& path) const noexcept
    {
      return At {path} (_root);
    }
  }
