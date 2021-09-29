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

#include <we/type/signature/specialize.hpp>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace
      {
        template<typename P>
        class apply_field : public boost::static_visitor<>
        {
        public:
          apply_field (P p)
            : _p (p)
          {}

          void operator() (std::pair<std::string, std::string>& f) const
          {
            _p._field (f);
          }
          void operator() (structured_type& s) const
          {
            _p._field_struct (s);
          }
        private:
          P _p;
        };

        class mapper
        {
        public:
          mapper (std::unordered_map<std::string, std::string> const& m)
            : _m (m)
          {}
          void _struct (structured_type& s) const
          {
            s.first = map (s.first);

            for (field_type& f : s.second)
            {
              boost::apply_visitor (apply_field<mapper> (*this), f);
            }
          }
          void _field (std::pair<std::string, std::string>& f) const
          {
            f.second = map (f.second);
          }
          void _field_struct (structured_type& s) const
          {
            _struct (s);
          }

        private:
          const std::unordered_map<std::string, std::string>& _m;

          std::string const& map (std::string const& x) const
          {
            const std::unordered_map<std::string, std::string>::const_iterator
              pos (_m.find (x));

            return (pos == _m.end()) ? x : pos->second;
          }
        };
      }

      void specialize
        ( structured_type& s
        , std::unordered_map<std::string, std::string> const& m
        )
      {
        mapper (m)._struct (s);
      }
    }
  }
}
