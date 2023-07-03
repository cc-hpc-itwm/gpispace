// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/signature.hpp>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      template<typename P>
        class traverse_field : public ::boost::static_visitor<>
      {
      public:
        traverse_field (P p)
          : _p (p)
        {}

        void operator() (std::pair<std::string, std::string> const& f) const
        {
          _p._field (f);
        }
        void operator() (structured_type const& s) const
        {
          _p._field_struct (s);
        }
      private:
        P _p;
      };

      template<typename P>
        void traverse (P p, field_type const& field)
      {
        ::boost::apply_visitor (traverse_field<P> (p), field);
      }
      template<typename P>
        void traverse (P p, structured_type const& s)
      {
        p._struct (s);
      }
    }
  }
}
