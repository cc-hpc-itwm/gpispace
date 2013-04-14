// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/show.hpp>

#include <we/type/value/signature/name_of.hpp>

#include <iostream>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      class visitor_show : public boost::static_visitor<std::ostream&>
      {
      public:
        visitor_show (std::ostream& os)
          : _os (os)
        {}

        std::ostream& operator() (const control& c) const
        {
          return _os << "[]";
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
        std::ostream& operator() (const std::list<value_type>& l) const
        {
          return print_container<std::list<value_type> >
            ( "(", ", ", ")", boost::ref (l)
            , boost::bind (&visitor_show::print_value, this, _1)
            );
        }
        std::ostream&
        operator() (const std::map<value_type, value_type>& m) const
        {
          return print_container<std::map<value_type, value_type> >
            ( "[", ", ", "]", boost::ref (m)
            , boost::bind (&visitor_show::print_map_item, this, _1)
            );
        }
        std::ostream& operator() (const std::vector<value_type>& v) const
        {
          return print_container<std::vector<value_type> >
            ( "(", ", ", ")", boost::ref (v)
            , boost::bind (&visitor_show::print_value, this, _1)
            );
        }
        std::ostream& operator() (const std::set<value_type>& s) const
        {
          return print_container<std::set<value_type> >
            ( "{", ", ", "}", boost::ref (s)
            , boost::bind (&visitor_show::print_value, this, _1)
            );
        }
        std::ostream& operator() (const structured_type& m) const
        {
          return print_container<structured_type>
            ( "[", ", ", "]", boost::ref (m)
            , boost::bind (&visitor_show::print_struct_item, this, _1)
            );
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
          ( const std::string& open
          , const std::string& sep
          , const std::string& close
          , const C& c
          , const boost::function<void (const typename C::value_type&)>& f
          ) const
        {
          _os << name_of<C> (c) << " " << open;
          bool first (true);
          BOOST_FOREACH (const typename C::value_type& x, c)
          {
            if (!first)
            {
              _os << sep;
            }
            f (x);
            first = false;
          }
          return _os << close;
        }
      };
    }
  }
}

std::ostream& operator<< ( std::ostream& os
                         , const pnet::type::value::value_type& v
                         )
{
  const std::ios_base::fmtflags ff (os.flags());
  os << std::showpoint << std::boolalpha;
  boost::apply_visitor (pnet::type::value::visitor_show (os), v);
  os.flags (ff);
  return os;
}
