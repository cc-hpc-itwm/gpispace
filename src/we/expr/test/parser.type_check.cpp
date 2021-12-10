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

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <we/exception.hpp>
#include <we/expr/eval/context.hpp>
#include <we/expr/parse/parser.hpp>
#include <we/type/value/boost/test/printer.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>

#include <util-generic/ostream/modifier.hpp>
#include <util-generic/join.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <functional>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <stack>
#include <unordered_map>

namespace expr
{
  namespace type
  {
    namespace
    {
      struct Expression : fhg::util::ostream::modifier
      {
        template<typename E>
          Expression (E const& e)
            : _ast ( parse::parser::DisableConstantFolding{}
                   , str (::boost::format ("%1%") % e)
                   )
        {}

        Type type() const
        {
          return _ast.type_check_all();
        }

        //! prints the ast
        virtual std::ostream& operator() (std::ostream& os) const override
        {
          for (auto const& node : _ast)
          {
            os << node;
          }

          return os;
        }

        expr::parse::node::type const& last() const
        {
          auto pos (_ast.begin());

          if (pos == _ast.end())
          {
            throw std::logic_error ("empty ast");
          }

          while (std::next (pos) != _ast.end())
          {
            ++pos;
          }

          return *pos;
        }

        //! prints the evaluated expression, useful in order to get an
        //! flexible expression. For example with
        //!   Expression e ("double (4)")
        //!   require (e.string() == "double(4)")
        //!   require (e.expression() == "4.00000")
        //! we have that
        //!   format ("List (%1%)") % e.string()
        //! fails to parse but
        //!   format ("List (%1%)") % e.expression()
        //! correctly parses.
        std::string expression() const
        {
          std::ostringstream oss;
          oss << pnet::type::value::show (_ast.eval_all());
          return oss.str();
        }

      private:
        parse::parser _ast;
      };

      BOOST_AUTO_TEST_CASE (expression_leads_to_flexible_input)
      {
        Expression const expression ("double (4)");

        //! For random doubles (that might be integral values) an
        //! explicit cast is required. But then the printed AST is not
        //! useable in any context, e.g. 'List (double (4))' fails to
        //! parse. To evaluate and print the value leads to a flexible
        //! input that can be used in any parser context.
        BOOST_REQUIRE_EQUAL (expression.string(), "double(4)");
        BOOST_REQUIRE_EQUAL (expression.expression(), "4.00000");
      }

      template< typename T
              , typename Generator = fhg::util::testing::random<T>
              >
        struct RandomExpression : public fhg::util::ostream::modifier
      {
        template<typename... Args>
          RandomExpression (Args&&... args)
            : _format (std::forward<Args> (args)...)
        {}
        virtual std::ostream& operator() (std::ostream& os) const override
        {
          return os
            << Expression ( ::boost::format (_format)
                          % ::boost::io::group (std::boolalpha, _generator())
                          ).expression()
            ;
        }

      private:
        std::string _format;
        Generator _generator;
      };

      template< typename T
              , typename Generator = fhg::util::testing::random<T>
              > RandomExpression<T, Generator> random_expression();
      template<> RandomExpression<int> random_expression<int>()
      {
        return "%1%";
      }
      template<> RandomExpression<unsigned int> random_expression<unsigned int>()
      {
        return "%1%u";
      }
      template<> RandomExpression<long> random_expression<long>()
      {
        return "%1%l";
      }
      template<> RandomExpression<unsigned long> random_expression<unsigned long>()
      {
        return "%1%ul";
      }
      template<> RandomExpression<double> random_expression<double>()
      {
        //! \note explicit cast as random might return an integral number
        return "double (%1%)";
      }
      template<> RandomExpression<float> random_expression<float>()
      {
        return "%1%f";
      }
      template<> RandomExpression<char> random_expression<char>()
      {
        return "'%1%'";
      }
      template<> RandomExpression<bool> random_expression<bool>()
      {
        return "%1%";
      }
      namespace
      {
        struct IdentifierWithoutLeadingUnderscore
        {
          using Generator = fhg::util::testing::random<std::string>;

          std::string const operator()() const
          {
            return _generator
              (Generator::identifier_without_leading_underscore());
          }

          Generator _generator;
        };
      }
      template<> RandomExpression<std::string, IdentifierWithoutLeadingUnderscore>
        random_expression<std::string, IdentifierWithoutLeadingUnderscore>()
      {
        return R"EOS("%1%")EOS";
      }
      namespace
      {
        std::string random_bitset()
        {
          using fhg::util::testing::random;
          random<unsigned long> random_ulong;
          auto N (random<int>{} (100));

          bitsetofint::type bs;

          while (N --> 0)
          {
            bs.ins (random_ulong());
          }

          return Expression (bs).expression();
        }
        std::string random_bytearray()
        {
          fhg::util::testing::random<std::string> rand_string;

          return Expression (we::type::bytearray (rand_string())).expression();
        }
      }

      struct TypeAndValue
      {
      public:
        TypeAndValue (Type type, std::string value)
          : _type (type)
          , _value (value)
        {}

        Type const& type() const
        {
          return _type;
        }
        std::string const& value() const
        {
          return _value;
        }
      private:
        Type _type;
        std::string _value;
      };

      std::ostream& operator<< (std::ostream& os, TypeAndValue const& tv)
      {
        return os << tv.value() << " :: " << tv.type();
      }


      template <typename Input, typename Exception>
        void require_inference_exception
          ( Input const& input
          , Exception const& exception
          )
      {
        Expression expression (input);

        fhg::util::testing::require_exception
          ( [&]()
            {
              (void) expression.type();
            }
          , fhg::util::testing::make_nested
            ( exception::type::error
              ( ::boost::format ("In '%1%'")
              % expression.last() // assume the last node causes the error
              )
            , exception
            )
          );
      }

      template<typename Input>
        void require_type_to
          ( Input const& input
          , Type const& expected
          )
      {
        BOOST_REQUIRE_EQUAL
          ( Expression (input).type()
          , expected
          );
      }

      std::vector<TypeAndValue> const types_and_values_random
        { {Boolean{}, random_expression<bool>().string()}
        , {Char{}, random_expression<char>().string()}
        , {String{}, random_expression<std::string, IdentifierWithoutLeadingUnderscore>().string()}
        , {Int{}, random_expression<int>().string()}
        , {UInt{}, random_expression<unsigned int>().string()}
        , {Long{}, random_expression<long>().string()}
        , {ULong{}, random_expression<unsigned long>().string()}
        , {Double{}, random_expression<double>().string()}
        , {Float{}, random_expression<float>().string()}
        , {Control{}, "[]"}
        , {Bitset{}, random_bitset()}
        , {Bytearray{}, random_bytearray()}
        };

      std::vector<std::string> const equality_and_non_equality
        {"==", "!="};

      template<typename Operation, typename Value>
        auto unary (Operation operation, Value value)
      {
        return ::boost::format ("%1% (%2%)")
          % operation
          % value
          ;
      }
      //! \todo type inference shall not depend on notation,
      //! e.g. infix or prefix
      template<typename Operation, typename LHS, typename RHS>
        auto binary_infix (Operation operation, LHS lhs, RHS rhs)
      {
        return ::boost::format ("%2% %1% %3%")
          % operation
          % lhs
          % rhs
          ;
      }
      template<typename Operation, typename LHS, typename RHS>
        auto binary_prefix (Operation operation, LHS lhs, RHS rhs)
      {
        return ::boost::format ("%1% (%2%, %3%)")
          % operation
          % lhs
          % rhs
          ;
      }

      Type collection (std::vector<Type> ts)
      {
        Types types;

        for (auto const& type : ts)
        {
          types._types.emplace (type);
        }

        if (auto type = types.singleton())
        {
          return *type;
        }

        return {types};
      }
    }

    BOOST_DATA_TEST_CASE
      ( error_eq_neq
      , types_and_values_random
      * types_and_values_random
      * equality_and_non_equality
      , lhs
      , rhs
      , op
      )
    {
      auto const expression (binary_infix (op, lhs.value(), rhs.value()));

      if (lhs.type() == rhs.type())
      {
        require_type_to (expression, Boolean{});
      }
      else
      {
        require_inference_exception
          ( expression
          , exception::type::error
            ( ::boost::format ("The left argument '%2%' of ' %1% ' has type '%3%' and the right argument '%4%' of ' %1% ' has type '%5%' but the types should be the same")
            % op
            % Expression (lhs.value())
            % lhs.type()
            % Expression (rhs.value())
            % rhs.type()
            )
          );
      }
    }

    BOOST_AUTO_TEST_CASE (basic_types)
    {
      require_type_to ("false", Boolean{});
      require_type_to ("true", Boolean{});
      require_type_to ("12", Int{});
      require_type_to ("42", Int{});
      require_type_to ("42U", UInt{});
      require_type_to ("3.14", Double{});
      require_type_to ("-2.7", Double{});
      require_type_to ("3.14f", Float{});
      require_type_to ("13L", Long{});
      require_type_to ("13UL", ULong{});
      require_type_to ("'x'", Char{});
      require_type_to ("\"Hello\"", String{});
      require_type_to ("[]", Control{});
      require_type_to ("{}", Bitset{});
    }

    BOOST_AUTO_TEST_CASE (min)
    {
      require_type_to ("min (0, 0)", Int{});
      require_type_to ("min (0U, 0U)", UInt{});
      require_type_to ("min (0L, 0L)", Long{});
      require_type_to ("min (0UL, 0UL)", ULong{});
      require_type_to ("min (0.0 0.0)", Double{});
      require_type_to ("min (0.0f, 0.0f)", Float{});
    }

    BOOST_AUTO_TEST_CASE (max)
    {
      require_type_to ("max (0, 0)", Int{});
      require_type_to ("max (0U, 0U)", UInt{});
      require_type_to ("max (0L, 0L)", Long{});
      require_type_to ("max (0UL, 0UL)", ULong{});
      require_type_to ("max (0.0 0.0)", Double{});
      require_type_to ("max (0.0f, 0.0f)", Float{});
    }

    BOOST_AUTO_TEST_CASE (list_basic)
    {
      require_type_to ("stack_top (List(42,23))" , Int{});
      require_type_to ("stack_empty (List(42,23))", Boolean{});
      require_type_to ("stack_size (List(42,23))", ULong{});
      require_type_to ("stack_push (List(42,23), 13)", List (Int{}));
      require_type_to ("stack_pop (List(42,23))", List (Int{}));
      require_type_to ("stack_join (List(42,23), List(4, 3))", List (Int{}));
    }

    BOOST_AUTO_TEST_CASE (basic_empty_list)
    {
      require_type_to ("stack_top (List())", Any());
      require_type_to ("stack_empty (List())", Boolean{});
      require_type_to ("stack_size (List())", ULong{});
      require_type_to ("stack_push (List(), 13)", List (Int{}));
      require_type_to ("stack_join (List(), List(4, 3))", List (Int{}));
      require_type_to ("stack_pop (List())", List {Any()});
    }

    BOOST_AUTO_TEST_CASE (log)
    {
      require_type_to ("log(3.14)", Double{});
      require_type_to ("log(3.14f)", Float{});
    }

    BOOST_AUTO_TEST_CASE (floor)
    {
      require_type_to ("floor(3.14)", Double{});
      require_type_to ("floor(3.14f)", Float{});
    }

    BOOST_AUTO_TEST_CASE (ceil)
    {
      require_type_to ("ceil(3.14)", Double{});
      require_type_to ("ceil(3.14f)", Float{});
    }

    BOOST_AUTO_TEST_CASE (round)
    {
      require_type_to ("round(3.14)", Double{});
      require_type_to ("round(3.14f)", Float{});
    }

    BOOST_AUTO_TEST_CASE (sin)
    {
      require_type_to ("sin(3.14)", Double{});
      require_type_to ("sin(3.14f)", Float{});
    }

    BOOST_AUTO_TEST_CASE (cos)
    {
      require_type_to ("cos(3.14)", Double{});
      require_type_to ("cos(3.14f)", Float{});
    }

    BOOST_AUTO_TEST_CASE (sqrt)
    {
      require_type_to ("sqrt(3.14)", Double{});
      require_type_to ("sqrt(3.14f)", Float{});
    }

    BOOST_AUTO_TEST_CASE (to_int)
    {
      require_type_to ("int(13)", Int{});
      require_type_to ("int(13U)", Int{});
      require_type_to ("int(42L)", Int{});
      require_type_to ("int(42UL)", Int{});
      require_type_to ("int(3.14)", Int{});
      require_type_to ("int(3.14f)", Int{});
    }

    BOOST_AUTO_TEST_CASE (to_uint)
    {
      require_type_to ("uint(13)", UInt{});
      require_type_to ("uint(13U)", UInt{});
      require_type_to ("uint(42L)", UInt{});
      require_type_to ("uint(42UL)", UInt{});
      require_type_to ("uint(3.14)", UInt{});
      require_type_to ("uint(3.14f)", UInt{});
    }

    BOOST_AUTO_TEST_CASE (to_long)
    {
      require_type_to ("long(13)", Long{});
      require_type_to ("long(13U)", Long{});
      require_type_to ("long(42L)", Long{});
      require_type_to ("long(42UL)", Long{});
      require_type_to ("long(3.14)", Long{});
      require_type_to ("long(3.14f)", Long{});
    }

    BOOST_AUTO_TEST_CASE (to_ulong)
    {
      require_type_to ("ulong(13)", ULong{});
      require_type_to ("ulong(13U)", ULong{});
      require_type_to ("ulong(42L)", ULong{});
      require_type_to ("ulong(42UL)", ULong{});
      require_type_to ("ulong(3.14)", ULong{});
      require_type_to ("ulong(3.14f)", ULong{});
    }

    BOOST_AUTO_TEST_CASE (to_float)
    {
      require_type_to ("float(13)", Float{});
      require_type_to ("float(13U)", Float{});
      require_type_to ("float(42L)", Float{});
      require_type_to ("float(42UL)", Float{});
      require_type_to ("float(3.14)", Float{});
      require_type_to ("float(3.14f)", Float{});
    }

    BOOST_AUTO_TEST_CASE (to_double)
    {
      require_type_to ("double(13)", Double{});
      require_type_to ("double(13U)", Double{});
      require_type_to ("double(42L)", Double{});
      require_type_to ("double(42UL)", Double{});
      require_type_to ("double(3.14)", Double{});
      require_type_to ("double(3.14f)", Double{});
    }

    BOOST_AUTO_TEST_CASE (struct_fields)
    {
      require_type_to ("${s} := Struct [a := 42] ; ${s.a}", Int{});
      require_type_to ("${s} := Struct [a := 42] ; ${s.a} := 13", Int{});
      require_type_to ("${s} := Struct [a := 12] ; ${s.x} := false; ${s.x}", Boolean{});
    }

    BOOST_AUTO_TEST_CASE (or_boolean_table)
    {
      require_type_to ("true || true", Boolean{});
      require_type_to ("true || false", Boolean{});
      require_type_to ("false || true", Boolean{});
      require_type_to ("false || false", Boolean{});

      require_type_to ("true :or: true", Boolean{});
      require_type_to ("true :or: false", Boolean{});
      require_type_to ("false :or: true", Boolean{});
      require_type_to ("false :or: false", Boolean{});
    }

    BOOST_AUTO_TEST_CASE (or_integral)
    {
      require_type_to ("0 | 0", Int{});
      require_type_to ("0 | 1", Int{});
      require_type_to ("1 | 1", Int{});
      require_type_to ("1 | 0", Int{});
      require_type_to ("1 | 2", Int{});
      require_type_to ("2 | 1", Int{});

      require_type_to ("0U | 0U", UInt{});
      require_type_to ("0U | 1U", UInt{});
      require_type_to ("1U | 1U", UInt{});
      require_type_to ("1U | 0U", UInt{});
      require_type_to ("1U | 2U", UInt{});
      require_type_to ("2U | 1U", UInt{});

      require_type_to ("0L | 0L", Long{});
      require_type_to ("0L | 1L", Long{});
      require_type_to ("1L | 1L", Long{});
      require_type_to ("1L | 0L", Long{});
      require_type_to ("1L | 2L", Long{});
      require_type_to ("2L | 1L", Long{});

      require_type_to ("0UL | 0UL", ULong{});
      require_type_to ("0UL | 1UL", ULong{});
      require_type_to ("1UL | 1UL", ULong{});
      require_type_to ("1UL | 0UL", ULong{});
      require_type_to ("1UL | 2UL", ULong{});
      require_type_to ("2UL | 1UL", ULong{});
    }


    BOOST_AUTO_TEST_CASE (and_boolean_table)
    {
      require_type_to ("true && true", Boolean{});
      require_type_to ("true && false", Boolean{});
      require_type_to ("false && true", Boolean{});
      require_type_to ("false && false", Boolean{});

      require_type_to ("true :and: true", Boolean{});
      require_type_to ("true :and: false", Boolean{});
      require_type_to ("false :and: true", Boolean{});
      require_type_to ("false :and: false", Boolean{});
    }

    BOOST_AUTO_TEST_CASE (and_integral)
    {
      require_type_to ("0 & 0", Int{});
      require_type_to ("0 & 1", Int{});
      require_type_to ("1 & 1", Int{});
      require_type_to ("1 & 0", Int{});
      require_type_to ("1 & 2", Int{});
      require_type_to ("2 & 1", Int{});
      require_type_to ("2 & 3", Int{});

      require_type_to ("0U & 0U", UInt{});
      require_type_to ("0U & 1U", UInt{});
      require_type_to ("1U & 1U", UInt{});
      require_type_to ("1U & 0U", UInt{});
      require_type_to ("1U & 2U", UInt{});
      require_type_to ("2U & 1U", UInt{});
      require_type_to ("2U & 3U", UInt{});

      require_type_to ("0L & 0L", Long{});
      require_type_to ("0L & 1L", Long{});
      require_type_to ("1L & 1L", Long{});
      require_type_to ("1L & 0L", Long{});
      require_type_to ("1L & 2L", Long{});
      require_type_to ("2L & 1L", Long{});
      require_type_to ("2L & 3L", Long{});

      require_type_to ("0UL & 0UL", ULong{});
      require_type_to ("0UL & 1UL", ULong{});
      require_type_to ("1UL & 1UL", ULong{});
      require_type_to ("1UL & 0UL", ULong{});
      require_type_to ("1UL & 2UL", ULong{});
      require_type_to ("2UL & 1UL", ULong{});
      require_type_to ("2UL & 3UL", ULong{});
    }

    BOOST_AUTO_TEST_CASE (not_has_type_Boolean)
    {
      require_type_to ("!true", Boolean{});
      require_type_to ("!false", Boolean{});
      require_type_to ("!!true", Boolean{});
      require_type_to ("!!false", Boolean{});
    }

    namespace
    {
      void type_check_compare_ops
        (std::string lhs, std::string rhs)
      {
        require_type_to (::boost::format ("%1% < %2%") % lhs % rhs, Boolean{});
        require_type_to (::boost::format ("%1% <= %2%") % lhs % rhs, Boolean{});
        require_type_to (::boost::format ("%1% > %2%") % lhs % rhs, Boolean{});
        require_type_to (::boost::format ("%1% >= %2%") % lhs % rhs, Boolean{});

        require_type_to (::boost::format ("%1% :lt: %2%") % lhs % rhs, Boolean{});
        require_type_to (::boost::format ("%1% :le: %2%") % lhs % rhs, Boolean{});
        require_type_to (::boost::format ("%1% :gt: %2%") % lhs % rhs, Boolean{});
        require_type_to (::boost::format ("%1% :ge: %2%") % lhs % rhs, Boolean{});
      }

      void type_check_equality_ops (std::string lhs, std::string rhs)
      {
        require_type_to (::boost::format ("%1% != %2%") % lhs % rhs, Boolean{});
        require_type_to (::boost::format ("%1% == %2%") % lhs % rhs, Boolean{});

        require_type_to (::boost::format ("%1% :ne: %2%") % lhs % rhs, Boolean{});
        require_type_to (::boost::format ("%1% :eq: %2%") % lhs % rhs, Boolean{});
      }
    }

    BOOST_AUTO_TEST_CASE (token_cmp)
    {
#define CHECK(_lhs, _rhs)                       \
      type_check_compare_ops (#_lhs, #_rhs);    \
      type_check_equality_ops (#_lhs, #_rhs)

      CHECK ('a', 'a');
      CHECK ('a', 'b');
      CHECK ('b', 'a');
      CHECK ("\"\"", "\"\"");
      CHECK ("\"\"", "\"a\"");
      CHECK ("\"a\"", "\"a\"");
      CHECK ("\"a\"", "\"b\"");
      CHECK ("\"a\"", "\"ab\"");
      CHECK ("\"a\"", "\"\"");
      CHECK ("\"b\"", "\"a\"");
      CHECK ("\"ab\"", "\"a\"");
      CHECK (true, true);
      CHECK (false, true);
      CHECK (true, false);
      CHECK (0, 0);
      CHECK (0, 1);
      CHECK (1, 0);
      CHECK (0U, 0U);
      CHECK (0U, 1U);
      CHECK (1U, 0U);
      CHECK (0L, 0L);
      CHECK (0L, 1L);
      CHECK (1L, 0L);
      CHECK (0UL, 0UL);
      CHECK (0UL, 1UL);
      CHECK (1UL, 0UL);
#undef CHECK

      type_check_compare_ops ("0.0", "0.0");
      type_check_compare_ops ("0.0", "1.0");
      type_check_compare_ops ("1.0", "0.0");
      type_check_compare_ops ("0.0f", "0.0f");
      type_check_compare_ops ("0.0f", "1.0f");
      type_check_compare_ops ("1.0f", "0.0f");

      type_check_equality_ops ("{}", "{}");
      type_check_equality_ops ("{}", "bitset_insert {} 1UL");
      type_check_equality_ops ("bitset_insert {} 1UL", "{}");
      type_check_equality_ops ("bitset_insert {} 1UL", "bitset_insert {} 2UL");
      type_check_equality_ops ( "bitset_insert (bitset_insert {} 1UL) 2UL"
                              , "bitset_insert {} 2UL"
                              );
      type_check_equality_ops ( "bitset_insert (bitset_insert {} 1UL) 2UL"
                              , "bitset_insert (bitset_insert {} 2UL) 1UL"
                              );

      type_check_equality_ops ("y()", "y()");
      type_check_equality_ops ("y(4)", "y()");
      type_check_equality_ops ("y()", "y(4)");
      type_check_equality_ops ("y(4)", "y(4)");

      type_check_equality_ops ("Struct[]", "Struct[]");
      type_check_equality_ops ("Struct[a:=0]", "Struct[a:=0]");
      type_check_equality_ops ("Struct[a:=0]", "Struct[a:=1]");
    }

    BOOST_AUTO_TEST_CASE (token_add)
    {
      require_type_to ("\"\" + \"\"", String{});
      require_type_to ("\"a\" + \"\"", String{});
      require_type_to ("\"a\" + \"a\"", String{});
      require_type_to ("\"ab\" + \"a\"", String{});

      require_type_to ("0 + 0", Int{});
      require_type_to ("0 + 1", Int{});
      require_type_to ("1 + 0", Int{});

      require_type_to ("0U + 0U", UInt{});
      require_type_to ("0U + 1U", UInt{});
      require_type_to ("1U + 0U", UInt{});

      require_type_to ("0L + 0L", Long{});
      require_type_to ("0L + 1L", Long{});
      require_type_to ("1L + 0L", Long{});

      require_type_to ("0UL + 0UL", ULong{});
      require_type_to ("0UL + 1UL", ULong{});
      require_type_to ("1UL + 0UL", ULong{});

      require_type_to ("0.0 + 0.0", Double{});
      require_type_to ("0.0 + 1.0", Double{});
      require_type_to ("1.0 + 0.0", Double{});
      require_type_to ("1.0 + 1.0", Double{});

      require_type_to ("0.0f + 0.0f", Float{});
      require_type_to ("0.0f + 1.0f", Float{});
      require_type_to ("1.0f + 0.0f", Float{});
      require_type_to ("1.0f + 1.0f", Float{});
    }

    BOOST_AUTO_TEST_CASE (token_mul)
    {
      require_type_to ("0 * 0", Int{});
      require_type_to ("0 * 1", Int{});
      require_type_to ("1 * 0", Int{});
      require_type_to ("1 * 1", Int{});
      require_type_to ("1 * 2", Int{});
      require_type_to ("2 * 1", Int{});

      require_type_to ("0U * 0U", UInt{});
      require_type_to ("0U * 1U", UInt{});
      require_type_to ("1U * 0U", UInt{});
      require_type_to ("1U * 1U", UInt{});
      require_type_to ("1U * 2U", UInt{});
      require_type_to ("2U * 1U", UInt{});

      require_type_to ("0L * 0L", Long{});
      require_type_to ("0L * 1L", Long{});
      require_type_to ("1L * 0L", Long{});
      require_type_to ("1L * 1L", Long{});
      require_type_to ("1L * 2L", Long{});
      require_type_to ("2L * 1L", Long{});

      require_type_to ("0UL * 0UL", ULong{});
      require_type_to ("0UL * 1UL", ULong{});
      require_type_to ("1UL * 0UL", ULong{});
      require_type_to ("1UL * 1UL", ULong{});
      require_type_to ("1UL * 2UL", ULong{});
      require_type_to ("2UL * 1UL", ULong{});

      require_type_to ("0.0 * 0.0", Double{});
      require_type_to ("0.0 * 1.0", Double{});
      require_type_to ("1.0 * 0.0", Double{});
      require_type_to ("1.0 * 1.0", Double{});
      require_type_to ("1.0 * 2.0", Double{});
      require_type_to ("2.0 * 1.0", Double{});

      require_type_to ("0.0f * 0.0f", Float{});
      require_type_to ("0.0f * 1.0f", Float{});
      require_type_to ("1.0f * 0.0f", Float{});
      require_type_to ("1.0f * 1.0f", Float{});
      require_type_to ("1.0f * 2.0f", Float{});
      require_type_to ("2.0f * 1.0f", Float{});
    }

    BOOST_AUTO_TEST_CASE (token_min)
    {
      require_type_to ("min (0, 0)", Int{});
      require_type_to ("min (0, 1)", Int{});
      require_type_to ("min (1, 0)", Int{});
      require_type_to ("min (1, 1)", Int{});
      require_type_to ("min (1, 2)", Int{});
      require_type_to ("min (2, 1)", Int{});

      require_type_to ("min (0U, 0U)", UInt{});
      require_type_to ("min (0U, 1U)", UInt{});
      require_type_to ("min (1U, 0U)", UInt{});
      require_type_to ("min (1U, 1U)", UInt{});
      require_type_to ("min (1U, 2U)", UInt{});
      require_type_to ("min (2U, 1U)", UInt{});

      require_type_to ("min (0L, 0L)", Long{});
      require_type_to ("min (0L, 1L)", Long{});
      require_type_to ("min (1L, 0L)", Long{});
      require_type_to ("min (1L, 1L)", Long{});
      require_type_to ("min (1L, 2L)", Long{});
      require_type_to ("min (2L, 1L)", Long{});

      require_type_to ("min (0UL, 0UL)", ULong{});
      require_type_to ("min (0UL, 1UL)", ULong{});
      require_type_to ("min (1UL, 0UL)", ULong{});
      require_type_to ("min (1UL, 1UL)", ULong{});
      require_type_to ("min (1UL, 2UL)", ULong{});
      require_type_to ("min (2UL, 1UL)", ULong{});

      require_type_to ("min (0.0, 0.0)", Double{});
      require_type_to ("min (0.0, 1.0)", Double{});
      require_type_to ("min (1.0, 0.0)", Double{});
      require_type_to ("min (1.0, 1.0)", Double{});
      require_type_to ("min (1.0, 2.0)", Double{});
      require_type_to ("min (2.0, 1.0)", Double{});

      require_type_to ("min (0.0f, 0.0f)", Float{});
      require_type_to ("min (0.0f, 1.0f)", Float{});
      require_type_to ("min (1.0f, 0.0f)", Float{});
      require_type_to ("min (1.0f, 1.0f)", Float{});
      require_type_to ("min (1.0f, 2.0f)", Float{});
      require_type_to ("min (2.0f, 1.0f)", Float{});
    }

    BOOST_AUTO_TEST_CASE (token_max)
    {
      require_type_to ("max (0, 0)", Int{});
      require_type_to ("max (0, 1)", Int{});
      require_type_to ("max (1, 0)", Int{});
      require_type_to ("max (1, 1)", Int{});
      require_type_to ("max (1, 2)", Int{});
      require_type_to ("max (2, 1)", Int{});

      require_type_to ("max (0U, 0U)", UInt{});
      require_type_to ("max (0U, 1U)", UInt{});
      require_type_to ("max (1U, 0U)", UInt{});
      require_type_to ("max (1U, 1U)", UInt{});
      require_type_to ("max (1U, 2U)", UInt{});
      require_type_to ("max (2U, 1U)", UInt{});

      require_type_to ("max (0L, 0L)", Long{});
      require_type_to ("max (0L, 1L)", Long{});
      require_type_to ("max (1L, 0L)", Long{});
      require_type_to ("max (1L, 1L)", Long{});
      require_type_to ("max (1L, 2L)", Long{});
      require_type_to ("max (2L, 1L)", Long{});

      require_type_to ("max (0UL, 0UL)", ULong{});
      require_type_to ("max (0UL, 1UL)", ULong{});
      require_type_to ("max (1UL, 0UL)", ULong{});
      require_type_to ("max (1UL, 1UL)", ULong{});
      require_type_to ("max (1UL, 2UL)", ULong{});
      require_type_to ("max (2UL, 1UL)", ULong{});

      require_type_to ("max (0.0, 0.0)", Double{});
      require_type_to ("max (0.0, 1.0)", Double{});
      require_type_to ("max (1.0, 0.0)", Double{});
      require_type_to ("max (1.0, 1.0)", Double{});
      require_type_to ("max (1.0, 2.0)", Double{});
      require_type_to ("max (2.0, 1.0)", Double{});

      require_type_to ("max (0.0f, 0.0f)", Float{});
      require_type_to ("max (0.0f, 1.0f)", Float{});
      require_type_to ("max (1.0f, 0.0f)", Float{});
      require_type_to ("max (1.0f, 1.0f)", Float{});
      require_type_to ("max (1.0f, 2.0f)", Float{});
      require_type_to ("max (2.0f, 1.0f)", Float{});
    }

    BOOST_AUTO_TEST_CASE (token_sub)
    {
      require_type_to ("0 - 0", Int{});
      require_type_to ("1 - 0", Int{});
      require_type_to ("0 - 1", Int{});

      require_type_to ("0L - 0L", Long{});
      require_type_to ("1L - 0L", Long{});
      require_type_to ("0L - 1L", Long{});

      require_type_to ("0.0 - 0.0", Double{});
      require_type_to ("0.0 - 1.0", Double{});
      require_type_to ("1.0 - 1.0", Double{});

      require_type_to ("0.0f - 0.0f", Float{});
      require_type_to ("0.0f - 1.0f", Float{});
      require_type_to ("1.0f - 1.0f", Float{});

      require_type_to ("0U - 0U", UInt{});
      require_type_to ("1U - 0U", UInt{});
      require_type_to ("2U - 1U", UInt{});

      require_type_to ("0UL - 0UL", ULong{});
      require_type_to ("1UL - 0UL", ULong{});
      require_type_to ("2UL - 1UL", ULong{});
    }

    BOOST_AUTO_TEST_CASE (token_divint)
    {
      require_type_to ("0 div 1", Int{});
      require_type_to ("1 div 1", Int{});
      require_type_to ("2 div 2", Int{});
      require_type_to ("2 div 1", Int{});

      require_type_to ("0U div 1U", UInt{});
      require_type_to ("1U div 1U", UInt{});
      require_type_to ("2U div 2U", UInt{});
      require_type_to ("2U div 1U", UInt{});

      require_type_to ("0L div 1L", Long{});
      require_type_to ("1L div 1L", Long{});
      require_type_to ("2L div 2L", Long{});
      require_type_to ("2L div 1L", Long{});

      require_type_to ("0UL div 1UL", ULong{});
      require_type_to ("1UL div 1UL", ULong{});
      require_type_to ("2UL div 2UL", ULong{});
      require_type_to ("2UL div 1UL", ULong{});
    }

    BOOST_AUTO_TEST_CASE (token_modint)
    {
      require_type_to ("0 mod 1", Int{});
      require_type_to ("1 mod 1", Int{});
      require_type_to ("2 mod 2", Int{});
      require_type_to ("2 mod 1", Int{});
      require_type_to ("1 mod 2", Int{});
      require_type_to ("5 mod 3", Int{});

      require_type_to ("0U mod 1U", UInt{});
      require_type_to ("1U mod 1U", UInt{});
      require_type_to ("2U mod 2U", UInt{});
      require_type_to ("2U mod 1U", UInt{});
      require_type_to ("1U mod 2U", UInt{});
      require_type_to ("5U mod 3U", UInt{});

      require_type_to ("0L mod 1L", Long{});
      require_type_to ("1L mod 1L", Long{});
      require_type_to ("2L mod 2L", Long{});
      require_type_to ("2L mod 1L", Long{});
      require_type_to ("1L mod 2L", Long{});
      require_type_to ("5L mod 3L", Long{});

      require_type_to ("0UL mod 1UL", ULong{});
      require_type_to ("1UL mod 1UL", ULong{});
      require_type_to ("2UL mod 2UL", ULong{});
      require_type_to ("2UL mod 1UL", ULong{});
      require_type_to ("1UL mod 2UL", ULong{});
      require_type_to ("5UL mod 3UL", ULong{});
    }

    BOOST_AUTO_TEST_CASE (token_div)
    {
      require_type_to ("0.0 / 1.0", Double{});
      require_type_to ("1.0 / 1.0", Double{});
      require_type_to ("2.0 / 1.0", Double{});
      require_type_to ("2.0 / 2.0", Double{});

      require_type_to ("0.0f / 1.0f", Float{});
      require_type_to ("1.0f / 1.0f", Float{});
      require_type_to ("2.0f / 1.0f", Float{});
      require_type_to ("2.0f / 2.0f", Float{});
    }

    BOOST_AUTO_TEST_CASE (token_pow)
    {
      require_type_to ("1.0 ** 0.0", Double{});
      require_type_to ("1.0 ** 1.0", Double{});

      require_type_to ("1.0f ** 0.0f", Float{});
      require_type_to ("1.0f ** 1.0f", Float{});
      require_type_to ("1.0f ** 2.0f", Float{});
      require_type_to ("2.0f ** 0.0f", Float{});
      require_type_to ("2.0f ** 1.0f", Float{});
      require_type_to ("2.0f ** 2.0f", Float{});
    }

    BOOST_AUTO_TEST_CASE (token_neg)
    {
      require_type_to ("-3.14", Double{});
      require_type_to ("-2.71f", Float{});
      require_type_to ("-42", Int{});
      require_type_to ("-13L", Long{});
    }

    BOOST_AUTO_TEST_CASE (token_abs)
    {
      require_type_to ("abs (-3.14)", Double{});
      require_type_to ("abs (-2.71f)", Float{});
      require_type_to ("abs (-42)", Int{});
      require_type_to ("abs (-13L)", Long{});
    }

    BOOST_AUTO_TEST_CASE (token_floor)
    {
      require_type_to ("floor (13)", Int{});
      require_type_to ("floor (13U)", UInt{});
      require_type_to ("floor (42L)", Long{});
      require_type_to ("floor (42UL)", ULong{});
      require_type_to ("floor (3.14)", Double{});
      require_type_to ("floor (2.71f)", Float{});
    }

    BOOST_AUTO_TEST_CASE (token_ceil)
    {
      require_type_to ("ceil (13)", Int{});
      require_type_to ("ceil (13U)", UInt{});
      require_type_to ("ceil (42L)", Long{});
      require_type_to ("ceil (42UL)", ULong{});
      require_type_to ("ceil (3.14)", Double{});
      require_type_to ("ceil (2.71f)", Float{});
    }

    BOOST_AUTO_TEST_CASE (token_round)
    {
      require_type_to ("round (13)", Int{});
      require_type_to ("round (13U)", UInt{});
      require_type_to ("round (42L)", Long{});
      require_type_to ("round (42UL)", ULong{});
      require_type_to ("round (3.14)", Double{});
      require_type_to ("round (2.71f)", Float{});
    }

    BOOST_AUTO_TEST_CASE (integral_followed_f_is_float)
    {
      require_type_to ("0f", Float{});
      require_type_to ("-1f", Float{});
      require_type_to ("1f", Float{});
      require_type_to ("42f", Float{});

      auto random_int (fhg::util::testing::random<int>{});

      for (int i (0); i < 100; ++i)
      {
        require_type_to (::boost::format ("%1%f") % random_int(), Float{});
      }
    }

    BOOST_AUTO_TEST_CASE (token_bitset_count)
    {
      require_type_to("bitset_count ({0 0 0 0 0 0 0 0 0 0 0 65536})", ULong{});
      require_type_to("bitset_count ({2099204 738871880977536 594475511590158864})", ULong{});
      require_type_to("bitset_count ({2305878227945521152 0 34359738368 2097156 144115188075856128 0})", ULong{});
      require_type_to("bitset_count ({1125899906842624 0 0 2684356608 2305983746702049280})", ULong{});
    }

    BOOST_AUTO_TEST_CASE (token_bitset_fromhex_tohex)
    {
      auto const bs ("{0 0 0 0 128}");

      require_type_to
        (::boost::format ("bitset_tohex (%1%)") % bs, String{});

      require_type_to
        (::boost::format ("bitset_fromhex (bitset_tohex (%1%))") % bs, Bitset{});
    }

    BOOST_AUTO_TEST_CASE (token_bitset_logical)
    {
      auto const l ("{0 0 0 0 0 0 0 0 0 0 0 65536}");
      auto const r ("{2099204 738871880977536 594475511590158864}");
      require_type_to
        (::boost::format ("bitset_or (%1%, %2%)") % l % r, Bitset{});
      require_type_to
        (::boost::format ("bitset_and (%1%, %2%)") % l % r, Bitset{});
      require_type_to
        (::boost::format ("bitset_xor (%1%, %2%)") % l % r, Bitset{});
    }

    BOOST_AUTO_TEST_CASE (token_bitset_ins_del_is_elem)
    {
      auto const bs ("{0 0 0 0 128}");
      auto const elem ("128UL");
      require_type_to
        (::boost::format ("bitset_insert (%1%, %2%)") % bs % elem, Bitset{});

      require_type_to
        (::boost::format ("bitset_is_element (%1%, %2%)") % bs % elem, Boolean{});

      require_type_to
        (::boost::format ("bitset_delete (%1%, %2%)") % bs % elem, Bitset{});
    }

    BOOST_AUTO_TEST_CASE (empty_list_has_type_list_any)
    {
      require_type_to
        ( "List()"
        , List {Any()}
        );
    }

    BOOST_DATA_TEST_CASE
      ( singleton_list_has_type_of_element
      , types_and_values_random
      , tv
      )
    {
      require_type_to
        ( ::boost::format ("List (%1%)") % tv.value()
        , List (tv.type())
        );
    }

    BOOST_DATA_TEST_CASE
      ( homogeneous_list_has_type_of_elements
      , types_and_values_random * types_and_values_random
      , tvA
      , tvB
      )
    {
      require_type_to
        ( ::boost::format ("List (%1%, %2%)") % tvA.value() % tvB.value()
        , List (collection ({tvA.type(), tvB.type()}))
        );
    }

    BOOST_AUTO_TEST_CASE (stack_empty_of_empty_stack_has_type_boolean)
    {
      require_type_to
        ( "stack_empty (List())"
        , Boolean{}
        );
    }
    BOOST_DATA_TEST_CASE
      ( stack_empty_of_non_empty_stack_has_type_boolean
      , types_and_values_random
      , tv
      )
    {
      require_type_to
        ( ::boost::format ("stack_empty (List (%1%))")
        % fhg::util::join
          ( std::vector<std::string>
              (fhg::util::testing::random<int>{} (100, 1), tv.value())
          , ", "
          )
        , Boolean{}
        );
    }

    BOOST_AUTO_TEST_CASE (stack_size_of_empty_stack_has_type_ulong)
    {
      require_type_to
        ( "stack_size (List())"
        , ULong{}
        );
    }
    BOOST_DATA_TEST_CASE
      ( stack_size_of_non_empty_stack_has_type_type_ULong
      , types_and_values_random
      , tv
      )
    {
      require_type_to
        ( ::boost::format ("stack_size (List (%1%))")
        % fhg::util::join
          ( std::vector<std::string>
              (fhg::util::testing::random<int>{} (100, 1), tv.value())
          , ", "
          )
        , ULong{}
        );
    }

    BOOST_DATA_TEST_CASE
      ( tokens_stack_pop
      , types_and_values_random
      , tv
      )
    {
      require_type_to
        ( ::boost::format ("stack_pop (List (%1%, %1%))") % tv.value()
        , List (tv.type())
        );
    }

    BOOST_DATA_TEST_CASE
      ( tokens_stack_top
      , types_and_values_random
      , tv
      )
    {
      require_type_to
        ( ::boost::format ("stack_top (List(%1%, %1%))") % tv.value()
        , tv.type()
        );
    }

    BOOST_DATA_TEST_CASE
      ( tokens_stack_join
      , types_and_values_random
      * types_and_values_random
      * types_and_values_random
      , valA
      , valB
      , valC
      )
    {
      require_type_to
        ( ::boost::format ("stack_join (List (%1%, %2%), List (%3%))")
          % valA.value()
          % valB.value()
          % valC.value()
        ,  List (collection ({valA.type(), valB.type(), valC.type()}))
        );
    }

    BOOST_AUTO_TEST_CASE (set_basic)
    {
      require_type_to ("set_insert (Set {13}, 42)", Set {Int{}});
      require_type_to ("set_erase (Set {13}, 42)", Set {Int{}});
      require_type_to ("set_is_element (Set {13}, 13)", Boolean{});
      require_type_to ("set_pop (Set {13})", Set {Int{}});
      require_type_to ("set_top (Set {13})", Int{});
      require_type_to ("set_empty (Set {13})", Boolean{});
      require_type_to ("set_size (Set {13})", ULong{});
      require_type_to ("set_is_subset (Set {13}, Set {13, 42})", Boolean{});
    }

    BOOST_AUTO_TEST_CASE (empty_set)
    {
      require_type_to ("set_insert (Set {}, 42)", Set (Int{}));
      require_type_to ("set_erase (Set {}, 42)", Set {Any()});
      require_type_to ("set_erase (Set {1}, 42)", Set {Int{}});
      require_type_to ("set_is_element (Set {}, 13)", Boolean{});
      require_type_to ("set_pop (Set {})", Set {Any()});
      require_type_to ("set_top (Set {})", Any());
      require_type_to ("set_empty (Set {})", Boolean{});
      require_type_to ("set_size (Set {})", ULong{});
      require_type_to ("set_is_subset (Set {}, Set {})", Boolean{});
      require_type_to ("set_is_subset (Set {13}, Set {})", Boolean{});
      require_type_to ("set_is_subset (Set {}, Set {42})", Boolean{});
    }

    BOOST_AUTO_TEST_CASE (set_empty_set)
    {
      require_type_to
        ( "Set{}"
        , Set {Any()}
        );
      require_type_to ("set_empty (Set{})", Boolean{});
      require_type_to ("set_size (Set{})", ULong{});
      require_type_to ("set_is_subset (Set{}, Set{})", Boolean{});
    }

    BOOST_DATA_TEST_CASE
      ( set_singleton_set
      , types_and_values_random
      , x
      )
    {
      require_type_to
        ( ::boost::format ("Set {%1%}") % x.value()
        , Set (x.type())
        );
    }

    BOOST_DATA_TEST_CASE
      ( set_multiple_elements
      , types_and_values_random * types_and_values_random
      , x
      , y
      )
    {
      require_type_to
        ( ::boost::format ("Set {%1%, %2%}") % x.value() % y.value()
        , Set (collection ({x.type(), y.type()}))
        );
    }

    BOOST_DATA_TEST_CASE
      ( set_empty
      , types_and_values_random
      , x
      )
    {
      require_type_to
        ( ::boost::format ("set_empty (Set {%1%})") % x.value()
        , Boolean{}
        );
    }

    BOOST_DATA_TEST_CASE
      ( set_size
      , types_and_values_random
      , x
      )
    {
      require_type_to
        ( ::boost::format ("set_size (Set {%1%})") % x.value()
        , ULong{}
        );
    }

    BOOST_DATA_TEST_CASE
      ( tokens_set_subset
      , types_and_values_random
      , x
      )
    {
      require_type_to
        ( ::boost::format ("set_is_subset (Set {%1%, %2%}, Set {%3%})")
          % x.value()
          % x.value()
          % x.value()
        , Boolean{}
        );
    }

    BOOST_DATA_TEST_CASE
      ( tokens_set_pop
      , types_and_values_random
      , x
      )
    {
      require_type_to
        ( ::boost::format ("set_pop (Set {%1%})") % x.value()
        , Set (x.type())
        );
    }

    namespace
    {
      template<typename T> auto random_expression_op_expr (std::string op)
      {
        return binary_infix
          (op, random_expression<T>(), random_expression<T>());
      }

      template<typename T> auto random_min_max_expr (std::string op)
      {
        return binary_prefix
          (op, random_expression<T>(), random_expression<T>());
      }

      std::vector<std::string> const integral_ops = {"&", "|", "div", "mod"};
      std::vector<std::string> const numeric_ops = {"+", "-", "*"};
      std::vector<std::string> const real_ops = {"**", "/"};
      std::vector<std::string> const min_max_ops = {"min", "max"};
    }

    BOOST_DATA_TEST_CASE
      ( random_integral_ops
      , integral_ops
      , op
      )
    {
      require_type_to (random_expression_op_expr<int> (op), Int{});
      require_type_to (random_expression_op_expr<unsigned int> (op), UInt{});
      require_type_to (random_expression_op_expr<long> (op), Long{});
      require_type_to (random_expression_op_expr<unsigned long> (op), ULong{});
    }

    BOOST_DATA_TEST_CASE
      ( random_expression_ops
      , numeric_ops
      , op
      )
    {
      require_type_to (random_expression_op_expr<int> (op), Int{});
      require_type_to (random_expression_op_expr<unsigned int> (op), UInt{});
      require_type_to (random_expression_op_expr<long> (op), Long{});
      require_type_to (random_expression_op_expr<unsigned long> (op), ULong{});
      require_type_to (random_expression_op_expr<double> (op), Double{});
      require_type_to (random_expression_op_expr<float> (op), Float{});
    }

    BOOST_DATA_TEST_CASE
      ( random_real_ops
      , real_ops
      , op
      )
    {
      require_type_to (random_expression_op_expr<double> (op), Double{});
      require_type_to (random_expression_op_expr<float> (op), Float{});
    }

    BOOST_DATA_TEST_CASE
      ( random_min_max_ops
      , min_max_ops
      , op
      )
    {
      require_type_to (random_min_max_expr<int> (op), Int{});
      require_type_to (random_min_max_expr<unsigned int> (op), UInt{});
      require_type_to (random_min_max_expr<long> (op), Long{});
      require_type_to (random_min_max_expr<unsigned long> (op), ULong{});
      require_type_to (random_min_max_expr<double> (op), Double{});
      require_type_to (random_min_max_expr<float> (op), Float{});
    }

    namespace
    {
      template<typename T>
        auto random_math_func (std::string op)
      {
        return unary (op, random_expression<T>());
      }

      std::vector<std::string> const math_funcs = {"floor", "ceil", "round"};
      std::vector<std::string> const real_math_funcs = {"sin", "cos", "sqrt", "log"};
    }

    BOOST_DATA_TEST_CASE
      ( random_math_funcs
      , math_funcs
      , math_func
      )
    {
      require_type_to (random_math_func<int> (math_func), Int{});
      require_type_to (random_math_func<unsigned int> (math_func), UInt{});
      require_type_to (random_math_func<long> (math_func), Long{});
      require_type_to (random_math_func<unsigned long> (math_func), ULong{});
      require_type_to (random_math_func<double> (math_func), Double{});
      require_type_to (random_math_func<float> (math_func), Float{});
    }

    BOOST_DATA_TEST_CASE
      ( random_real_math_funcs
      , real_math_funcs
      , math_func
      )
    {
      require_type_to (random_math_func<double> (math_func), Double{});
      require_type_to (random_math_func<float> (math_func), Float{});
    }

    BOOST_AUTO_TEST_CASE (random_math_abs)
    {
      require_type_to (random_math_func<int> ("abs"), Int{});
      require_type_to (random_math_func<long> ("abs"), Long{});
      require_type_to (random_math_func<double> ("abs"), Double{});
      require_type_to (random_math_func<float> ("abs"), Float{});
    }

    namespace
    {
      struct CastFuncAndType
      {
      public:
        CastFuncAndType (std::string cast_func, Type type)
          : _cast_func (cast_func)
          , _type (type)
        {}

        std::string const& cast_func() const { return _cast_func; }
        Type const& casted_type() const { return _type; }

      private:
        std::string _cast_func;
        Type _type;
      };

      std::ostream& operator<< (std::ostream& os, CastFuncAndType const& tv)
      {
        return os << tv.cast_func() << " :: " << tv.casted_type();
      }

      std::vector<CastFuncAndType> cast_func_types {
        {"int", Int{}},
        {"uint", UInt{}},
        {"long", Long{}},
        {"ulong", ULong{}},
        {"double", Double{}},
        {"float", Float{}},
      };
    }

    BOOST_DATA_TEST_CASE
      ( random_casts
      , cast_func_types
      , cast_func_type
      )
    {
      auto const cast_func (cast_func_type.cast_func());
      auto const casted_type (cast_func_type.casted_type());
      require_type_to (random_math_func<int> (cast_func), casted_type);
      require_type_to (random_math_func<unsigned int> (cast_func), casted_type);
      require_type_to (random_math_func<long> (cast_func), casted_type);
      require_type_to (random_math_func<unsigned long> (cast_func), casted_type);
      require_type_to (random_math_func<double> (cast_func), casted_type);
      require_type_to (random_math_func<float> (cast_func), casted_type);
    }

    namespace
    {
      std::vector<TypeAndValue> const non_numeric_types =
        { {Boolean{}, "true"}
        , {Char{}, "'X'"}
        , {String{}, "\"Hello\""}
        , {Control{}, "[]"}
        , {Bitset{}, "{}"}
        };

      std::vector<TypeAndValue> const numeric_types =
        { {Int{}, "42"}
        , {UInt{}, "1729U"}
        , {Long{}, "13L"}
        , {ULong{}, "9UL"}
        , {Double{}, "3.14000"}
        , {Float{}, "2.71000f"}
        };
    }

    BOOST_DATA_TEST_CASE
      ( error_casts
      , cast_func_types * non_numeric_types
      , cast_func_type
      , value
      )
    {
      require_inference_exception
        ( ::boost::format ("%1% (%2%)")
          % cast_func_type.cast_func()
          % value.value()
        , exception::type::error
          ( ::boost::format
            ("argument '%2%' of '%1%' has type '%3%' but is not of kind 'Number' == {'int', 'long', 'unsigned int', 'unsigned long', 'float', 'double'}")
          % cast_func_type.cast_func()
          % Expression (value.value())
          % value.type()
          )
        );
    }

    namespace
    {
      std::vector<std::string> const numeric_ops_without_add = {"-", "*"};
    }

    BOOST_DATA_TEST_CASE
      ( error_numeric_ops
      , numeric_ops_without_add * non_numeric_types * non_numeric_types
      , op
      , lhs
      , rhs
      )
    {
      require_inference_exception
        ( binary_infix (op, lhs.value(), rhs.value())
        , exception::type::error
          ( ::boost::format
            ("left argument '%2%' of ' %1% ' has type '%3%' but is not of kind 'Number' == {'int', 'long', 'unsigned int', 'unsigned long', 'float', 'double'}")
          % op
          % Expression (lhs.value())
          % lhs.type()
          )
        );
    }


    BOOST_DATA_TEST_CASE
      ( error_numeric_op_non_numeric
      , numeric_ops_without_add * numeric_types * non_numeric_types
      , op
      , lhs
      , rhs
      )
    {
      require_inference_exception
        ( binary_infix (op, lhs.value(), rhs.value())
        , exception::type::error
          ( ::boost::format
            ("right argument '%2%' of ' %1% ' has type '%3%' but is not of kind 'Number' == {'int', 'long', 'unsigned int', 'unsigned long', 'float', 'double'}")
          % op
          % Expression (rhs.value())
          % rhs.type()
          )
        );
    }

    BOOST_DATA_TEST_CASE
      ( error_min_max_ops
      , min_max_ops * non_numeric_types * non_numeric_types
      , op
      , lhs
      , rhs
      )
    {
      require_inference_exception
        ( binary_prefix (op, lhs.value(), rhs.value())
        , exception::type::error
          ( ::boost::format
            ("left argument '%2%' of '%1%' has type '%3%' but is not of kind 'Number' == {'int', 'long', 'unsigned int', 'unsigned long', 'float', 'double'}")
          % op
          % Expression (lhs.value())
          % lhs.type()
          )
        );
    }

    BOOST_DATA_TEST_CASE
      ( error_min_max_op_non_numeric
      , min_max_ops * numeric_types * non_numeric_types
      , op
      , lhs
      , rhs
      )
    {
      require_inference_exception
        ( binary_prefix (op, lhs.value(), rhs.value())
        , exception::type::error
          ( ::boost::format ("right argument '%2%' of '%1%' has type '%3%' but is not of kind 'Number' == {'int', 'long', 'unsigned int', 'unsigned long', 'float', 'double'}")
          % op
          % Expression (rhs.value())
          % rhs.type()
          )
        );
    }

    namespace
    {
      std::vector<TypeAndValue> const non_integer_types =
        { {Boolean{}, "true"}
        , {Char{}, "'X'"}
        , {String{}, "\"Hello\""}
        , {Control{}, "[]"}
        , {Bitset{}, "{}"}
        , {Double{}, "3.14000"}
        , {Float{}, "2.71000f"}
        };

      std::vector<TypeAndValue> const integer_types =
        { {Int{}, "42"}
        , {UInt{}, "1729U"}
        , {Long{}, "13L"}
        , {ULong{}, "9UL"}
        };
    }

    BOOST_DATA_TEST_CASE
      ( error_integer_ops
      , integral_ops * non_integer_types * non_integer_types
      , op
      , lhs
      , rhs
      )
    {
      require_inference_exception
        ( binary_infix (op, lhs.value(), rhs.value())
        , exception::type::error
          ( ::boost::format ("left argument '%2%' of ' %1% ' has type '%3%' but is not of kind 'Integer' == {'int', 'long', 'unsigned int', 'unsigned long'}")
          % op
          % Expression (lhs.value())
          % lhs.type()
          )
        );
    }

    BOOST_DATA_TEST_CASE
      ( error_integer_op_non_inter
      , integral_ops * integer_types * non_integer_types
      , op
      , lhs
      , rhs
      )
    {
      require_inference_exception
        ( binary_infix (op, lhs.value(), rhs.value())
        , exception::type::error
          ( ::boost::format
            ("right argument '%2%' of ' %1% ' has type '%3%' but is not of kind 'Integer' == {'int', 'long', 'unsigned int', 'unsigned long'}")
          % op
          % Expression (rhs.value())
          % rhs.type()
          )
        );
    }

    namespace
    {
      std::vector<std::string> const boolean_bin_ops =
         { "&&"
         , "||"
         };
    }

    BOOST_DATA_TEST_CASE
      ( error_boolean_binary_op
      , boolean_bin_ops * types_and_values_random * types_and_values_random
      , op
      , lhs
      , rhs
      )
    {
      if (lhs.type() != Type {Boolean{}})
      {
        require_inference_exception
          ( binary_infix (op, lhs.value(), rhs.value())
          , exception::type::error
            ( ::boost::format
              ("left argument '%2%' of ' %1% ' has type '%3%' but requires type 'bool'")
            % op
            % Expression (lhs.value())
            % lhs.type()
            )
          );
      }
      else if (rhs.type() != Type {Boolean{}})
      {
        require_inference_exception
          ( binary_infix (op, lhs.value(), rhs.value())
          , exception::type::error
            ( ::boost::format
              ("right argument '%2%' of ' %1% ' has type '%3%' but requires type 'bool'")
            % op
            % Expression (rhs.value())
            % rhs.type()
            )
          );
      }
    }

    BOOST_AUTO_TEST_CASE (basic_map)
    {
      require_type_to ("map_assign (Map[42->false], 1, true)", Map (Int{}, Boolean{}));
      require_type_to ("map_unassign (Map[42->true], 42)", Map (Int{}, Boolean{}));
      require_type_to ("map_is_assigned (Map[42->false], 42)", Boolean{});
      require_type_to ("map_get_assignment (Map[42->true], 42)", Boolean{});
      require_type_to ("map_size (Map[42->true])", ULong{});
      require_type_to ("map_empty (Map[42->true])", Boolean{});
    }

    BOOST_AUTO_TEST_CASE (basic_empty_map)
    {
      require_type_to ("map_assign (Map[], 1, true)", Map (Int{}, Boolean{}));
      require_type_to ("map_unassign (Map[], 42)", Map {Any(), Any()});
      require_type_to ("map_unassign (map_assign (Map[], 1, true), 42)", Map (Int{}, Boolean{}));
      require_type_to ("map_is_assigned (Map[], 42)", Boolean{});
      require_type_to ("map_get_assignment (Map[], 42)", Any());
      require_type_to ("map_size (Map[])", ULong{});
      require_type_to ("map_empty (Map[])", Boolean{});
    }

    BOOST_AUTO_TEST_CASE (map_empty_init)
    {
      require_type_to
        ( "Map[]"
        , Map {Any(), Any()}
        );
    }

    BOOST_DATA_TEST_CASE
      ( map_init
      , types_and_values_random * types_and_values_random
      , lhs
      , rhs
      )
    {
      require_type_to
        ( ::boost::format ("Map [%1% -> %2%]") % lhs.value() % rhs.value()
        , Map (lhs.type(), rhs.type())
        );
    }

    BOOST_AUTO_TEST_CASE (basic_struct)
    {
      require_type_to
        ("${s} := Struct [a := 42] ; ${s.a}", Int{});
      require_type_to
        ("${s} := Struct [a := 42] ; ${s.a} := 13"
        , Int{}
        );
      require_type_to
        ("${s} := Struct [a := 12] ; ${s.x} := false; ${s.x}"
        , Boolean{}
        );
      require_type_to
        ("${s} := Struct [a := 12, b := false]"
        , Struct ({{"a", Int{}}, {"b", Boolean{}}})
        );
    }

    BOOST_DATA_TEST_CASE
      ( struct_field_can_be_written_directly
      , types_and_values_random
      , x
      )
    {
      require_type_to
        ( ::boost::format ("${s.x} := %1%") % x.value()
        , x.type()
        );
      require_type_to
        ( ::boost::format ("${s.x} := %1%; ${s}") % x.value()
        , Struct ({{"x", x.type()}})
        );
    }

    BOOST_DATA_TEST_CASE
      ( basic_struct_error
      , types_and_values_random
      , x
      )
    {
      require_inference_exception
        ( ::boost::format ("${s} := Struct [a := 12, b := false]; ${s} := %1%")
        % x.value()
        , fhg::util::testing::make_nested
          ( exception::type::error
            ( ::boost::format ("expr::type::Context::bind (${s}, '%1%')")
            % x.type()
            )
          , exception::type::error
            ( ::boost::format ("At ${s}: Can not assign a value of type '%1%' to a value of type '%2%'")
            % x.type()
            % Struct ({{"a", Int{}}, {"b", Boolean{}}})
            )
          )
        );
    }

    BOOST_DATA_TEST_CASE
      ( set_size_of_list_produces_error
      , types_and_values_random
      , tv
      )
    {
      require_inference_exception
        ( ::boost::format ("set_size (stack_push (List(), %1%))") % tv.value()
        , exception::type::error
          ( ::boost::format ("argument 'stack_push(List (), %1%)' of 'set_size' has type 'List [%2%]' but requires type 'Set'")
          % Expression (tv.value())
          % tv.type()
          )
        );
    }
  }
}
