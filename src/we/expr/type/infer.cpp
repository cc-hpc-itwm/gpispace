// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/expr/type/infer.hpp>

#include <we/expr/exception.hpp>
#include <we/expr/token/prop.hpp>
#include <we/type/value.hpp>

#include <util-generic/functor_visitor.hpp>
#include <util-generic/print_container.hpp>

#include <FMT/boost/variant.hpp>
#include <FMT/util-generic/join.hpp>
#include <FMT/we/expr/parse/node.hpp>
#include <FMT/we/expr/token/show.hpp>
#include <FMT/we/expr/type/Path.hpp>
#include <FMT/we/expr/type/Type.hpp>
#include <algorithm>
#include <exception>
#include <fmt/core.h>
#include <functional>
#include <iterator>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>

namespace expr
{
  namespace type
  {
    namespace
    {
      struct plus : private ::boost::static_visitor<Type>
      {
#define LITERAL(T)                                      \
        Type operator() (T const& lhs, T const&) const  \
        {                                               \
          return lhs;                                   \
        }

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

        Type operator() (List const& lhs, List const& rhs) const
        {
          return List
            (::boost::apply_visitor (*this, lhs._element, rhs._element));
        }

        Type operator() (Set const& lhs, Set const& rhs) const
        {
          return Set
            (::boost::apply_visitor (*this, lhs._element, rhs._element));
        }

        Type operator() (Map const& lhs, Map const& rhs) const
        {
          return Map
            ( ::boost::apply_visitor (*this, lhs._key, rhs._key)
            , ::boost::apply_visitor (*this, lhs._value, rhs._value)
            );
        }

        Type operator() (Struct const& lhs, Struct const& rhs) const
        {
          Struct::Fields sum;

          if (std::equal ( std::begin (lhs._fields), std::end (lhs._fields)
                         , std::begin (rhs._fields), std::end (rhs._fields)
                         , [&sum, this] (auto const& l, auto const& r)
                           {
                             return l._name == r._name
                               && is_singleton
                                  ( sum.emplace
                                    ( std::end (sum)
                                    , l._name
                                    , ::boost::apply_visitor
                                        (*this, l._type, r._type)
                                    )->_type
                                  )
                               ;
                           }
                         )
             )
          {
            return Struct (sum);
          }

          return Types {{lhs, rhs}};
        }

        Type operator() (Types const& lhs, Types const& rhs) const
        {
          Types sum;

          std::set_union ( std::begin (lhs._types), std::end (lhs._types)
                         , std::begin (rhs._types), std::end (rhs._types)
                         , std::inserter (sum._types, std::end (sum._types))
                         );

          if (auto type = sum.singleton())
          {
            return *type;
          }

          return {sum};
        }
        template<typename LHS>
          Type operator() (LHS const& lhs, Types const& rhs) const
        {
          return this->operator() (Types {lhs}, rhs);
        }
        template<typename RHS>
          Type operator() (Types const& lhs, RHS const& rhs) const
        {
          return this->operator() (lhs, Types {rhs});
        }
        template<typename LHS, typename RHS>
          Type operator() (LHS const& lhs, RHS const& rhs) const
        {
          return this->operator() (Types {lhs}, Types {rhs});
        }
      };

      Type operator+ (Type const& lhs, Type const& rhs)
      {
        return ::boost::apply_visitor (plus{}, lhs, rhs);
      }

      class visitor_value_to_type : private ::boost::static_visitor<Type>
      {
        template<typename Range>
          Type values_to_type (Range const& range) const
        {
          return std::accumulate
            ( std::begin (range), std::end (range)
            , Any()
            , [this] (auto type, auto const& value)
              {
                return type + ::boost::apply_visitor (*this, value);
              }
            );
        }

        using Value = ::pnet::type::value::value_type;

      public:
#define LITERAL(R,T...) Type operator() (T const&) const { return R{}; }

        LITERAL (Control, we::type::literal::control)
        LITERAL (Boolean, bool)
        LITERAL (Int, int)
        LITERAL (Long, long)
        LITERAL (UInt, unsigned int)
        LITERAL (ULong, unsigned long)
        LITERAL (Float, float)
        LITERAL (Double, double)
        LITERAL (Char, char)
        LITERAL (String, std::string)
        LITERAL (Bitset, bitsetofint::type)
        LITERAL (Bytearray, we::type::bytearray)

#undef LITERAL

        Type operator() (std::list<Value> const& list) const
        {
          return List (values_to_type (list));
        }

        Type operator() (std::set<Value> const& set) const
        {
          return Set (values_to_type (set));
        }

        Type operator() (std::map<Value, Value> const& map) const
        {
          struct Keys
          {
            struct KeysIt
            {
              [[nodiscard]] auto operator*() const -> Value const&
              {
                return _pos->first;
              }
              [[nodiscard]] auto operator++() -> KeysIt&
              {
                ++_pos;
                return *this;
              }
              [[nodiscard]] auto operator!= (KeysIt const& other) const -> bool
              {
                return _pos != other._pos;
              }
              std::map<Value, Value>::const_iterator _pos;
              std::map<Value, Value>::const_iterator _end;
            };

            [[nodiscard]] auto begin() const -> KeysIt
            {
              return KeysIt {std::cbegin (_map), std::cend (_map)};
            }
            [[nodiscard]] auto end() const -> KeysIt
            {
              return KeysIt {std::cend (_map), std::cend (_map)};
            }
            std::map<Value, Value> const& _map;
          };
          struct Vals
          {
            struct ValsIt
            {
              [[nodiscard]] auto operator*() const -> Value const&
              {
                return _pos->second;
              }
              [[nodiscard]] auto operator++() -> ValsIt&
              {
                ++_pos;
                return *this;
              }
              [[nodiscard]] auto operator!= (ValsIt const& other) const -> bool
              {
                return _pos != other._pos;
              }
              std::map<Value, Value>::const_iterator _pos;
              std::map<Value, Value>::const_iterator _end;
            };

            [[nodiscard]] auto begin() const -> ValsIt
            {
              return ValsIt {std::cbegin (_map), std::cend (_map)};
            }
            [[nodiscard]] auto end() const -> ValsIt
            {
              return ValsIt {std::cend (_map), std::cend (_map)};
            }
            std::map<Value, Value> const& _map;
          };

          return Map ( values_to_type (Keys {map})
                     , values_to_type (Vals {map})
                     );
        }

        Type operator() (::pnet::type::value::structured_type const& s) const
        {
          Struct::Fields field_types;

          for (auto const& field : s)
          {
            field_types.emplace_back
              (field.first, ::boost::apply_visitor (*this, field.second));
          }

          return Struct (field_types);
        }
      };

      struct Kind
      {
        std::string name;
        std::set<Type> types;
      };

      template<typename Description, typename Fun>
        auto when_kind (Description description, Kind kind, Type type, Fun fun)
        -> decltype (fun (type))
      {
        auto const types
          ( fhg::util::visit<Types>
            ( type
            , [] (Types const& ts)
              {
                return ts;
              }
            , [] (auto const& t)
              {
                return Types {t};
              }
            )
          );

        if (std::any_of ( std::begin (types._types), std::end (types._types)
                        , [&] (auto const& t)
                          {
                            return !kind.types.count (t);
                          }
                        )
           )
        {
          throw exception::type::error
            { fmt::format ( "{0} has type '{1}' but is not of kind '{2}' == {3}"
                          , description
                          , type
                          , kind.name
                          , fhg::util::print_container ("{'", "', '", "'}", kind.types)
                          )
            };
        }

        return fun (type);
      }

      static Kind const Number = { "Number"
                                 , { Int{}
                                   , Double{}
                                   , Float{}
                                   , Long{}
                                   , ULong{}
                                   , UInt{}
                                   }
                                 };
      static Kind const Signed = { "Signed"
                                 , { Int{}
                                   , Double{}
                                   , Float{}
                                   , Long{}
                                   }
                                 };
      static Kind const Integer =  { "Integer"
                                   , { Int{}
                                     , Long{}
                                     , ULong{}
                                     , UInt{}
                                     }
                                   };
      static Kind const Ord = { "Ordered"
                              , { Boolean{}
                                , Char{}
                                , String{}
                                , Int{}
                                , Double{}
                                , Float{}
                                , Long{}
                                , ULong{}
                                , UInt{}
                                }
                              };
      static Kind const Addable = { "Addable"
                                  , { Boolean{}
                                    , String{}
                                    , Int{}
                                    , Double{}
                                    , Float{}
                                    , Long{}
                                    , ULong{}
                                    , UInt{}
                                    }
                                  };
      static Kind const Fractional = { "Fractional"
                                     , { Double{}
                                       , Float{}
                                       }
                                     };

      template<typename Description, typename Required>
        std::string not_the_same
          ( Description description
          , Required required
          , Type type
          )
      {
        return fmt::format ( "{0} has type '{1}' but requires type '{2}'"
                           , description
                           , type
                           , required
                           );
      }

      template<typename Description>
        void require (Description description, Type type, Type required)
      {
        if (type != required)
        {
          throw exception::type::error
            (not_the_same (description, required, type));
        }
      }

      template<typename T, typename Description, typename Key, typename Fun>
        auto withT (Description description, Key key, Type type, Fun fun)
          -> decltype (fun (std::declval<T>()))
      {
        using Ret = decltype (fun (std::declval<T>()));

        return fhg::util::visit<Ret>
          ( type
          , [&] (T const& x)
            {
              return fun (x);
            }
          , [&] (auto const&) -> Ret
            {
              throw exception::type::error
                (not_the_same (description, key, type));
            }
          );
      }

      template<typename Description>
        Type list_element (Description description, Type type)
      {
        return withT<List>
          ( description, "List", type
          , [] (auto const& list)
            {
              return list._element;
            }
          );
      }
      template<typename Description>
        Type set_element (Description description, Type type)
      {
        return withT<Set>
          ( description, "Set", type
          , [] (auto const& set)
            {
              return set._element;
            }
          );
      }
      template<typename Description>
        Type map_key (Description description, Type type)
      {
        return withT<Map>
          ( description, "Map", type
          , [] (auto const& map)
            {
              return map._key;
            }
          );
      }
      template<typename Description>
        Type map_value (Description description, Type type)
      {
        return withT<Map>
          ( description, "Map", type
          , [] (auto const& map)
            {
              return map._value;
            }
          );
      }

      struct Const
      {
        Const (Type what)
          : _what (what)
        {}

        template<typename... Ts> Type operator() (Ts...) const
        {
          return _what;
        }

      private:
        Type _what;
      };

      struct Identity
      {
        template<typename T> Type operator() (T x) const
        {
          return x;
        }
      };

      struct Second
      {
        template<typename T, typename U> Type operator() (T, U x) const
        {
          return x;
        }
      };

      using Element = Identity;
      using Value = Second;

      class visitor_infer : private ::boost::static_visitor<Type>
      {
      private:
        Context& _context;

      public:
        visitor_infer (Context& context)
          : _context (context)
        {}

        Type operator() (pnet::type::value::value_type const& value) const
        {
          return ::boost::apply_visitor (visitor_value_to_type{}, value);
        }

        Type operator() (Path::Particles const& path) const
        {
          if (auto const type = _context.at (path))
          {
            return *type;
          }

          throw std::runtime_error
            {fmt::format ("Could not infer type of {}", Path {path})};
        }

        Type operator() (parse::node::unary_t const& u) const
        {
          auto const description
            {fmt::format ("argument '{}' of '{}'", u.child, u.token)};
          auto const child (::boost::apply_visitor (*this, u.child));

          auto const function
            ( [&] (auto arg, auto ret) -> Type
              {
                require (description, child, arg);

                return ret;
              }
            );
          auto const with_list
            ( [&] (auto fun)
              {
                return fun (list_element (description, child));
              }
            );
          auto const with_set
            ( [&] (auto fun)
              {
                return fun (set_element (description, child));
              }
            );
          auto const with_map
            ( [&] (auto fun)
              {
                return fun
                  ( map_key (description, child)
                  , map_value (description, child)
                  );
              }
            );
          auto const with_kind
            ( [&] (Kind kind, auto fun)
              {
                return when_kind (description, kind, child, fun);
              }
            );

          switch (u.token)
          {
          case token::_not: return function (Boolean{}, Boolean{});

          case token::_bitset_tohex:   return function (Bitset{}, String{});
          case token::_bitset_fromhex: return function (String{}, Bitset{});
          case token::_bitset_count:   return function (Bitset{}, ULong{});

          case token::_stack_empty: return with_list (Const (Boolean{}));
          case token::_stack_top:   return with_list (Element{});
          case token::_stack_pop:   return with_list (Const (child));
          case token::_stack_size:  return with_list (Const (ULong{}));

          case token::_set_empty: return with_set (Const (Boolean{}));
          case token::_set_top:   return with_set (Element{});
          case token::_set_pop:   return with_set (Const (child));
          case token::_set_size:  return with_set (Const (ULong{}));

          case token::_map_size:  return with_map (Const (ULong{}));
          case token::_map_empty: return with_map (Const (Boolean{}));

          case token::abs:
          case token::neg:
            return with_kind (Signed, Identity{});

          case token::_floor:
          case token::_ceil:
          case token::_round:
            return with_kind (Number, Identity{});

          case token::_sin:
          case token::_cos:
          case token::_sqrt:
          case token::_log:
            return with_kind (Fractional, Identity{});

          case token::_toint:    return with_kind (Number, Const (Int{}));
          case token::_tolong:   return with_kind (Number, Const (Long{}));
          case token::_touint:   return with_kind (Number, Const (UInt{}));
          case token::_toulong:  return with_kind (Number, Const (ULong{}));
          case token::_tofloat:  return with_kind (Number, Const (Float{}));
          case token::_todouble: return with_kind (Number, Const (Double{}));

          default: break;
          }

          throw std::logic_error
            { fmt::format ( "Unknown unary token '{}' in ''"
                          , u.token
                          , expr::parse::node::type {u}
                          )
            };
        }

        Type operator() (parse::node::binary_t const& b) const
        {
          auto const rhs (::boost::apply_visitor (*this, b.r));

          if (is_define (b.token))
          {
            return _context.bind (::boost::get<Path::Particles> (b.l), rhs);
          }

          auto const description_lhs
            {fmt::format ("left argument '{}' of '{}'", b.l, b.token)};
          auto const description_rhs
            {fmt::format ("right argument '{}' of '{}'", b.r, b.token)};

          auto const lhs (::boost::apply_visitor (*this, b.l));

          auto const function
            ( [&] (auto argl, auto argr, auto ret) -> Type
              {
                require (description_lhs, lhs, argl);
                require (description_rhs, rhs, argr);

                return ret;
              }
            );
          auto const with_kinds
            ( [&] (Kind kindl, Kind kindr, auto fun)
              {
                return when_kind
                  ( description_lhs, kindl, lhs
                  , [&, fun] (auto l)
                    {
                      return when_kind
                        ( description_rhs, kindr, rhs
                        , std::bind (fun, l, std::placeholders::_1)
                        );
                    }
                  );
              }
            );
          auto const with_set
            ( [&] (auto fun)
              {
                return fun (set_element (description_lhs, lhs));
              }
            );
          auto const with_sets
            ( [&] (auto fun)
              {
                return with_set
                  ( [&] (auto elementL)
                    {
                      return fun (elementL, set_element (description_rhs, rhs));
                    }
                  );
              }
            );
          auto const with_map
            ( [&] (auto fun)
              {
                return fun
                  ( map_key (description_lhs, lhs)
                  , map_value (description_lhs, lhs)
                  );
              }
            );
          auto const IfEqual
            ( [&] (auto l, auto r, auto fun)
              {
                if (l != Any() && r != Any() && l != r)
                {
                  throw exception::type::error
                    { fmt::format ( "The {0} has type '{1}' and the {2} has type '{3}' but the types should be the same"
                                  , description_lhs
                                  , l
                                  , description_rhs
                                  , r
                                  )
                    };
                }

                return fun (l);
              }
            );
          auto const IfEqualThen
            ( [&] (auto fun)
              {
                return std::bind
                  (IfEqual, std::placeholders::_1, std::placeholders::_2, fun);
              }
            );

          switch (b.token)
          {
          case token::_or_boolean:
          case token::_and_boolean:
            return function (Boolean{}, Boolean{}, Boolean{});

          case token::_bitset_insert:
          case token::_bitset_delete:
            return function (Bitset{}, ULong{}, Bitset{});
          case token::_bitset_is_element:
            return function (Bitset{}, ULong{}, Boolean{});
          case token::_bitset_or:
          case token::_bitset_and:
          case token::_bitset_xor:
            return function (Bitset{}, Bitset{}, Bitset{});

          case token::_or_integral:
          case token::_and_integral:
          case token::divint:
          case token::modint:
          case token::_powint:
            return with_kinds (Integer, Integer, IfEqualThen (Identity{}));

          case token::add:
            return with_kinds (Addable, Addable, IfEqualThen (Identity{}));

          case token::min:
          case token::max:
          case token::sub:
          case token::mul:
            return with_kinds (Number, Number, IfEqualThen (Identity{}));

          case token::div:
          case token::_pow:
            return with_kinds (Fractional, Fractional, IfEqualThen (Identity{}));

          case token::lt:
          case token::le:
          case token::gt:
          case token::ge:
            return with_kinds (Ord, Ord, IfEqualThen (Const (Boolean{})));

          case token::ne:
          case token::eq:
            return IfEqual (lhs, rhs, Const (Boolean{}));

          case token::_stack_push:
            return List (list_element (description_lhs, lhs) + rhs);

          case token::_stack_join:
            return List ( list_element (description_lhs, lhs)
                        + list_element (description_rhs, rhs)
                        );

          case token::_set_insert:
            return Set (set_element (description_lhs, lhs) + rhs);

          case token::_set_erase:
            return with_set (Const (lhs));

          case token::_set_is_element:
            return with_set (Const (Boolean{}));

          case token::_set_is_subset:
            return with_sets (Const (Boolean{}));

          case token::_map_unassign:
            return with_map (Const (lhs));

          case token::_map_is_assigned:
            return with_map (Const (Boolean{}));

          case token::_map_get_assignment:
            return with_map (Value{});

          default: break;
          }

          throw std::logic_error
            { fmt::format ( "Unknown binary token '{}' in '{}'"
                          , b.token
                          , expr::parse::node::type {b}
                          )
            };
        }

        Type operator() (parse::node::ternary_t const& t) const
        {
          auto const description_0
            { fmt::format ( "first argument '{}' of '{}'"
                          , t.child0
                          , t.token
                          )
            };
          auto const child0 (::boost::apply_visitor (*this, t.child0));
          auto const child1 (::boost::apply_visitor (*this, t.child1));
          auto const child2 (::boost::apply_visitor (*this, t.child2));

          switch (t.token)
          {
          case token::_map_assign:
            return Map ( map_key (description_0, child0) + child1
                       , map_value (description_0, child0) + child2
                       );

          default: break;
          }

          throw std::logic_error
            { fmt::format ( "Unknown ternary token '{}' in '{}'"
                          , t.token
                          , expr::parse::node::type {t}
                          )
            };
        }
      };
    }

    Type infer (Context& context, parse::node::type const& node)
    try
    {
      return ::boost::apply_visitor (visitor_infer (context), node);
    }
    catch (...)
    {
      std::throw_with_nested
        (exception::type::error {fmt::format ("In '{}'", node)});
    }
  }
}
