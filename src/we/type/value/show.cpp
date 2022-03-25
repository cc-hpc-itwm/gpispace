// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <we/type/value/show.hpp>

#include <util-generic/print_container.hpp>

#include <functional>
#include <iostream>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace
      {
        class visitor_show : public ::boost::static_visitor<std::ostream&>
        {
        public:
          visitor_show (std::ostream& os)
            : _os (os)
          {}

          std::ostream& operator() (std::list<value_type> const& l) const
          {
            return _os << fhg::util::print_container
              ( "List (", ", ", ")", l
              , std::bind ( &visitor_show::print_value, this
                          , std::placeholders::_1, std::placeholders::_2
                          )
              );
          }
          std::ostream&
          operator() (std::map<value_type, value_type> const& m) const
          {
            return _os << fhg::util::print_container
              ( "Map [", ", ", "]", m
              , std::bind ( &visitor_show::print_map_item, this
                          , std::placeholders::_1, std::placeholders::_2
                          )
              );
          }
          std::ostream& operator() (std::set<value_type> const& s) const
          {
            return _os << fhg::util::print_container
              ( "Set {", ", ", "}", s
              , std::bind ( &visitor_show::print_value, this
                          , std::placeholders::_1, std::placeholders::_2
                          )
              );
          }
          std::ostream& operator() (structured_type const& m) const
          {
            return _os << fhg::util::print_container
              ( "Struct [", ", ", "]", m
              , std::bind ( &visitor_show::print_struct_item, this
                          , std::placeholders::_1, std::placeholders::_2
                          )
              );
          }
          std::ostream& operator() (control const& c) const
          {
            return _os << c;
          }
          std::ostream& operator() (bool const& b) const
          {
            return _os << b;
          }
          std::ostream& operator() (int const& i) const
          {
            return _os << i;
          }
          std::ostream& operator() (long const& i) const
          {
            return _os << i << "L";
          }
          std::ostream& operator() (unsigned int const& i) const
          {
            return _os << i << "U";
          }
          std::ostream& operator() (unsigned long const& i) const
          {
            return _os << i << "UL";
          }
          std::ostream& operator() (float const& f) const
          {
            return _os << f << "f";
          }
          std::ostream& operator() (double const& d) const
          {
            return _os << d;
          }
          std::ostream& operator() (char const& c) const
          {
            return _os << "'" << c << "'";
          }
          std::ostream& operator() (std::string const& s) const
          {
            return _os << "\"" << s << "\"";
          }
          std::ostream& operator() (bitsetofint::type const& bs) const
          {
            return _os << bs;
          }
          std::ostream& operator() (we::type::bytearray const& ba) const
          {
            return _os << ba;
          }

        private:
          std::ostream& _os;

          std::ostream& print_value (std::ostream& os, value_type const& x) const
          {
            ::boost::apply_visitor (*this, x);
            return os;
          }
          std::ostream& print_map_item (std::ostream& os, std::pair<value_type, value_type> const& x) const
          {
            ::boost::apply_visitor (*this, x.first);
            os << " -> ";
            ::boost::apply_visitor (*this, x.second);
            return os;
          }
          std::ostream& print_struct_item
          (std::ostream& os, std::pair<std::string, value_type> const& x) const
          {
            os << x.first << " := ";
            ::boost::apply_visitor (*this, x.second);
            return os;
          }
        };
      }

      show::show (value_type const& value)
        : _value (value)
      {}
      std::ostream& show::operator() (std::ostream& os) const
      {
        const std::ios_base::fmtflags ff (os.flags());
        os << std::showpoint << std::boolalpha;
        ::boost::apply_visitor (visitor_show (os), _value);
        os.flags (ff);
        return os;
      }
      std::ostream& operator<< (std::ostream& os, show const& s)
      {
        return s (os);
      }
    }
  }
}
