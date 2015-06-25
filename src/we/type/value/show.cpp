// mirko.rahn@itwm.fraunhofer.de

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
        class visitor_show : public boost::static_visitor<std::ostream&>
        {
        public:
          visitor_show (std::ostream& os)
            : _os (os)
          {}

          std::ostream& operator() (const std::list<value_type>& l) const
          {
            return _os << fhg::util::print_container<decltype (l)>
              ( "List (", ", ", ")", l
              , std::bind ( &visitor_show::print_value, this
                          , std::placeholders::_1, std::placeholders::_2
                          )
              );
          }
          std::ostream&
          operator() (const std::map<value_type, value_type>& m) const
          {
            return _os << fhg::util::print_container<decltype (m)>
              ( "Map [", ", ", "]", m
              , std::bind ( &visitor_show::print_map_item, this
                          , std::placeholders::_1, std::placeholders::_2
                          )
              );
          }
          std::ostream& operator() (const std::set<value_type>& s) const
          {
            return _os << fhg::util::print_container<decltype (s)>
              ( "Set {", ", ", "}", s
              , std::bind ( &visitor_show::print_value, this
                          , std::placeholders::_1, std::placeholders::_2
                          )
              );
          }
          std::ostream& operator() (const structured_type& m) const
          {
            return _os << fhg::util::print_container<decltype (m)>
              ( "Struct [", ", ", "]", m
              , std::bind ( &visitor_show::print_struct_item, this
                          , std::placeholders::_1, std::placeholders::_2
                          )
              );
          }
          std::ostream& operator() (const control& c) const
          {
            return _os << c;
          }
          std::ostream& operator() (const bool& b) const
          {
            return _os << b;
          }
          std::ostream& operator() (const int& i) const
          {
            return _os << i;
          }
          std::ostream& operator() (const long& i) const
          {
            return _os << i << "L";
          }
          std::ostream& operator() (const unsigned int& i) const
          {
            return _os << i << "U";
          }
          std::ostream& operator() (const unsigned long& i) const
          {
            return _os << i << "UL";
          }
          std::ostream& operator() (const float& f) const
          {
            return _os << f << "f";
          }
          std::ostream& operator() (const double& d) const
          {
            return _os << d;
          }
          std::ostream& operator() (const char& c) const
          {
            return _os << "'" << c << "'";
          }
          std::ostream& operator() (const std::string& s) const
          {
            return _os << "\"" << s << "\"";
          }
          std::ostream& operator() (const bitsetofint::type& bs) const
          {
            return _os << bs;
          }
          std::ostream& operator() (const we::type::bytearray& ba) const
          {
            return _os << ba;
          }

        private:
          std::ostream& _os;

          std::ostream& print_value (std::ostream& os, const value_type& x) const
          {
            boost::apply_visitor (*this, x);
            return os;
          }
          std::ostream& print_map_item (std::ostream& os, const std::pair<value_type, value_type>& x) const
          {
            boost::apply_visitor (*this, x.first);
            os << " -> ";
            boost::apply_visitor (*this, x.second);
            return os;
          }
          std::ostream& print_struct_item
          (std::ostream& os, const std::pair<std::string, value_type>& x) const
          {
            os << x.first << " := ";
            boost::apply_visitor (*this, x.second);
            return os;
          }
        };
      }

      show::show (const value_type& value)
        : _value (value)
      {}
      std::ostream& show::operator() (std::ostream& os) const
      {
        const std::ios_base::fmtflags ff (os.flags());
        os << std::showpoint << std::boolalpha;
        boost::apply_visitor (visitor_show (os), _value);
        os.flags (ff);
        return os;
      }
      std::ostream& operator<< (std::ostream& os, const show& s)
      {
        return s (os);
      }
    }
  }
}
