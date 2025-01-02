// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/expr/type/AssignResult.hpp>

#include <we/expr/exception.hpp>

#include <util-generic/join.hpp>

#include <FMT/util-generic/join.hpp>
#include <FMT/we/expr/type/Path.hpp>
#include <FMT/we/expr/type/Type.hpp>
#include <fmt/core.h>
#include <iostream>
#include <iterator>

namespace expr
{
  namespace type
  {
    namespace
    {
      struct ScopedPathExtension
      {
        template<typename Extension>
          ScopedPathExtension (Path& path, Extension const& what)
            : _path (path)
        {
          _path.emplace_back (what);
        }
        ScopedPathExtension (ScopedPathExtension const&) = delete;
        ScopedPathExtension (ScopedPathExtension&&) = delete;
        ScopedPathExtension& operator= (ScopedPathExtension const&) = delete;
        ScopedPathExtension& operator= (ScopedPathExtension&&) = delete;
        ~ScopedPathExtension()
        {
          _path.pop_back();
        }

        Path& _path;
      };

      struct visitor_result_type : ::boost::static_visitor<Type>
      {
        visitor_result_type (Path& path)
          : _path (path)
        {}

        Path& _path;

        template<typename Key, typename LHS, typename RHS>
          Type deeper (Key const& key, LHS const& lhs, RHS const& rhs) const
        {
          ScopedPathExtension const scoped_path_extension (_path, key);

          return ::boost::apply_visitor (*this, lhs, rhs);
        }

#define LITERAL(T)                                        \
        Type operator() (T const&, T const& rhs) const    \
        {                                                 \
          return rhs;                                     \
        }                                                 \

        LITERAL (Control)
        LITERAL (Boolean)
        LITERAL (Int)
        LITERAL (Long)
        LITERAL (UInt)
        LITERAL (ULong)
        LITERAL (Float)
        LITERAL (Double)
        LITERAL (Char)
        LITERAL (String)
        LITERAL (Bitset)
        LITERAL (Bytearray)

#undef LITERAL

        Type operator() ( List const& lhs
                        , List const& rhs
                        ) const
        {
          return List
            (deeper ("LIST_ELEMENT_TYPE", lhs._element, rhs._element));
        }
        Type operator() ( Set const& lhs
                        , Set const& rhs
                        ) const
        {
          return Set
            (deeper ("SET_ELEMENT_TYPE", lhs._element, rhs._element));
        }
        Type operator() ( Map const& lhs
                        , Map const& rhs
                        ) const
        {
          return Map
            ( deeper ("MAP_KEY_TYPE", lhs._key, rhs._key)
            , deeper ("MAP_VALUE_TYPE", lhs._value, rhs._value)
            );
        }
        Type operator() ( Struct const& lhs
                        , Struct const& rhs
                        ) const
        {
          Struct::Fields fields;

          auto l (std::begin (lhs._fields));
          auto r (std::begin (rhs._fields));

          for (
              ; l != std::end (lhs._fields) && r != std::end (rhs._fields)
              ; ++l, ++r
              )
          {
            if (l->_name != r->_name)
            {
              throw exception::type::error
                { fmt::format
                    ( "Can not assign a value of type '{1}' to a value of type '{0}' at {2}: Missing field '{3}', found '{4}' instead"
                    , lhs
                    , rhs
                    , _path
                    , l->_name
                    , r->_name
                    )
                };
            }

            fields.emplace_back
              ( l->_name
              , deeper (l->_name, l->_type, r->_type)
              );
          }

          if (l != std::end (lhs._fields))
          {
            throw exception::type::error
              { fmt::format
                  ( "Can not assign a value of type '{1}' to a value of type '{0}' at {2}: Missing field(s) {{'{3}'}}"
                  , lhs
                  , rhs
                  , _path
                  , fhg::util::join
                    ( Struct::Fields {l, std::end (lhs._fields)}
                    , "', '"
                    , [] (auto& os, auto const& field) -> decltype (os)
                      {
                        return os << field._name;
                      }
                    )
                  )
              };
          }

          if (r != std::end (rhs._fields))
          {
            throw exception::type::error
              { fmt::format
                  ( "Can not assign a value of type '{1}' to a value of type '{0}' at {2}: Additional field(s) {{'{3}'}}"
                  , lhs
                  , rhs
                  , _path
                  , fhg::util::join
                    ( Struct::Fields {r, std::end (rhs._fields)}
                    , "', '"
                    , [] (auto& os, auto const& field) -> decltype (os)
                      {
                        return os << field._name;
                      }
                    )
                  )
              };
          }

          return Struct (fields);
        }

        template<typename LHS, typename RHS>
          void warn_overwritten (LHS const& lhs, RHS const& rhs) const
        {
          (void) lhs;
          (void) rhs;

          // std::clog
          //   << ( fmt::format
          //          ("At {0}: Type '{1}' overwritten with type '{2}'")
          //      % _path
          //      % lhs
          //      % rhs
          //      )
          //   << std::endl
          //   ;
        }
        template<typename RHS>
          void warn_multi_type (RHS const& rhs) const
        {
          (void) rhs;

          // std::clog
          //   << ( fmt::format ("At {0}: Multi-type '{1}'")
          //      % _path
          //      % rhs
          //      )
          //   << std::endl
          //   ;
        }

        Type operator() (Types const& lhs, Types const& rhs) const
        {
          if (lhs != rhs)
          {
            warn_overwritten (lhs, rhs);
          }

          if (rhs.is_multi())
          {
            warn_multi_type (rhs);
          }

          return rhs;
        }
        template<typename RHS>
          Type operator() (Types const& lhs, RHS const& rhs) const
        {
          warn_overwritten (lhs, rhs);

          return rhs;
        }
        template<typename LHS>
          Type operator() (LHS const& lhs, Types const& rhs) const
        {
          warn_overwritten (lhs, rhs);

          if (rhs.is_multi())
          {
            warn_multi_type (rhs);
          }

          return rhs;
        }

        template<typename LHS, typename RHS>
          Type operator() (LHS const& lhs, RHS const& rhs) const
        {
          throw exception::type::error
            { fmt::format
               ( "At {2}: Can not assign a value of type '{1}'"
                 " to a value of type '{0}'"
               , lhs
               , rhs
               , _path
               )
            };
        }
      };
    }

    Type assign_result (Path path, Type const& lhs, Type const& rhs)
    {
      return ::boost::apply_visitor (visitor_result_type {path}, lhs, rhs);
    }
  }
}
