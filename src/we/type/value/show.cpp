// Copyright (C) 2013-2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/value/show.hpp>

#include <gspc/util/print_container.hpp>

#include <functional>
#include <iostream>



    namespace gspc::pnet::type::value
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
            return _os << util::print_container
              ( "List (", ", ", ")", l
              , std::bind ( &visitor_show::print_value, this
                          , std::placeholders::_1, std::placeholders::_2
                          )
              );
          }
          std::ostream&
          operator() (std::map<value_type, value_type> const& m) const
          {
            return _os << util::print_container
              ( "Map [", ", ", "]", m
              , std::bind ( &visitor_show::print_map_item, this
                          , std::placeholders::_1, std::placeholders::_2
                          )
              );
          }
          std::ostream& operator() (std::set<value_type> const& s) const
          {
            return _os << util::print_container
              ( "Set {", ", ", "}", s
              , std::bind ( &visitor_show::print_value, this
                          , std::placeholders::_1, std::placeholders::_2
                          )
              );
          }
          std::ostream& operator() (structured_type const& m) const
          {
            return _os << util::print_container
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
          std::ostream& operator() (pnet::type::bitsetofint::type const& bs) const
          {
            return _os << bs;
          }
          std::ostream& operator() (we::type::bytearray const& ba) const
          {
            return _os << ba;
          }
          std::ostream& operator() (bigint_type const& i) const
          {
            return _os << i << "A";
          }
          std::ostream& operator() (we::type::shared const& s) const
          {
            return _os << s;
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
