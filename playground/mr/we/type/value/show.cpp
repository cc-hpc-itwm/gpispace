// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/show.hpp>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>

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
            return print_container<std::list<value_type> >
              ( "list", "(", ",", ")", boost::ref (l)
              , boost::bind (&visitor_show::print_value, this, _1)
              );
          }
          std::ostream&
          operator() (const std::map<value_type, value_type>& m) const
          {
            return print_container<std::map<value_type, value_type> >
              ( "map", "[", ",", "]", boost::ref (m)
              , boost::bind (&visitor_show::print_map_item, this, _1)
              );
          }
          std::ostream& operator() (const std::set<value_type>& s) const
          {
            return print_container<std::set<value_type> >
              ( "set", "{", ",", "}", boost::ref (s)
              , boost::bind (&visitor_show::print_value, this, _1)
              );
          }
          std::ostream& operator() (const structured_type& m) const
          {
            return print_container<structured_type>
              ( "struct", "[", ",", "]", boost::ref (m)
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

          template<typename C>
          std::ostream& print_container
          ( const std::string& header
          , const std::string& open
          , const std::string& sep
          , const std::string& close
          , const C& c
          , const boost::function<void (const typename C::value_type&)>& f
          ) const
          {
            _os << header << " " << open;
            bool first (true);
            BOOST_FOREACH (const typename C::value_type& x, c)
            {
              if (!first)
              {
                _os << sep << " ";
              }
              f (x);
              first = false;
            }
            return _os << close;
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
