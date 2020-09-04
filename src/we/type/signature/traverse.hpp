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

#include <we/type/signature.hpp>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      template<typename P>
        class traverse_field : public boost::static_visitor<>
      {
      public:
        traverse_field (P p)
          : _p (p)
        {}

        void operator() (const std::pair<std::string, std::string>& f) const
        {
          _p._field (f);
        }
        void operator() (const structured_type& s) const
        {
          _p._field_struct (s);
        }
      private:
        P _p;
      };

      template<typename P>
        void traverse (P p, const field_type& field)
      {
        boost::apply_visitor (traverse_field<P> (p), field);
      }
      template<typename P>
        void traverse (P p, const structured_type& s)
      {
        p._struct (s);
      }
    }
  }
}
