// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/expr/eval/context.hpp>
#include <we/expr/type/Context.hpp>
#include <we/type/Expression.hpp>

#include <boost/serialization/string.hpp>

#include <string>

namespace we
{
  namespace type
  {
    class MemoryBufferInfo
    {
    public:
      //! For deserialization only.
      MemoryBufferInfo();

      MemoryBufferInfo
        (std::string const& size, std::string const& alignment);

      unsigned long size (expr::eval::context const&) const;
      unsigned long alignment (expr::eval::context const&) const;

      void assert_correct_expression_types
        (expr::type::Context const&) const;

    private:
      Expression _size;
      Expression _alignment;

      friend class ::boost::serialization::access;

      template<typename Archive>
      void serialize (Archive & ar, unsigned int)
      {
        ar & _size;
        ar & _alignment;
      }
    };
  }
}
