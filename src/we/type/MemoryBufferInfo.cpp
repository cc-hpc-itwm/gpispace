// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/expr/parse/parser.hpp>
#include <we/type/MemoryBufferInfo.hpp>

#include <util-generic/nest_exceptions.hpp>

#include <boost/format.hpp>

#include <stdexcept>

namespace we
{
  namespace type
  {
    MemoryBufferInfo::MemoryBufferInfo() = default;

    MemoryBufferInfo::MemoryBufferInfo
        (std::string const& size, std::string const& alignment)
      : _size (size)
      , _alignment (alignment)
    {}

    unsigned long MemoryBufferInfo::size
      (expr::eval::context const& input) const
    {
      expr::eval::context context (input);
      return ::boost::get<unsigned long>
        (_size.ast().eval_all (context));
    }

    unsigned long MemoryBufferInfo::alignment
       (expr::eval::context const& input) const
     {
       expr::eval::context context (input);
       auto const alignment
         ( ::boost::get<unsigned long>
             (_alignment.ast().eval_all (context))
         );

       //note: equivalent to std::has_single_bit (c++ 20)
       auto const is_power_of_2 =
         [] (unsigned long n)
         {
           return n != 0 && (n & (n - 1)) == 0;
         };

       if (!is_power_of_2 (alignment))
       {
         throw std::runtime_error
           ("Invalid alignment expression. The alignment should be a power of 2!");
       }

       return alignment;
     }

    void MemoryBufferInfo::assert_correct_expression_types
      (expr::type::Context const& context) const
    {
      fhg::util::nest_exceptions<std::runtime_error>
        ( [&]
          {
            _size.assert_type (expr::type::ULong{}, context);
          }
        , str ( ::boost::format ("In the <size> expression '%1%'")
              % _size.expression()
              )
        );
      fhg::util::nest_exceptions<std::runtime_error>
        ( [&]
          {
            _alignment.assert_type (expr::type::ULong{}, context);
          }
        , str ( ::boost::format ("In the <alignment> expression '%1%'")
              % _alignment.expression()
              )
        );
    }
  }
}
