// mirko.rahn@itwm.fraunhofer.de

#include <we2/type/value/show.hpp>

#include <fhg/util/print_container.hpp>

#include <boost/bind.hpp>

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
            return fhg::util::print_container<std::list<value_type> >
              ( _os, "list ", "(", ",", ")", boost::ref (l)
              , boost::bind (&visitor_show::print_value, this, _1)
              );
          }
          std::ostream&
          operator() (const std::map<value_type, value_type>& m) const
          {
            return fhg::util::print_container<std::map<value_type, value_type> >
              ( _os, "map ", "[", ",", "]", boost::ref (m)
              , boost::bind (&visitor_show::print_map_item, this, _1)
              );
          }
          std::ostream& operator() (const std::set<value_type>& s) const
          {
            return fhg::util::print_container<std::set<value_type> >
              ( _os, "set ", "{", ",", "}", boost::ref (s)
              , boost::bind (&visitor_show::print_value, this, _1)
              );
          }
          std::ostream& operator() (const structured_type& m) const
          {
            return fhg::util::print_container<structured_type>
              ( _os, "struct ", "[", ",", "]", boost::ref (m)
              , boost::bind (&visitor_show::print_struct_item, this, _1)
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
          std::ostream& operator() (const bytearray::type& ba) const
          {
            return _os << ba;
          }

        private:
          std::ostream& _os;

          void print_value (const value_type& x) const
          {
            boost::apply_visitor (*this, x);
          }
          void print_map_item (const std::pair<value_type, value_type>& x) const
          {
            boost::apply_visitor (*this, x.first);
            _os << " -> ";
            boost::apply_visitor (*this, x.second);
          }
          void print_struct_item
          (const std::pair<std::string, value_type>& x) const
          {
            _os << x.first << " := ";
            boost::apply_visitor (*this, x.second);
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
