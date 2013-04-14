// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/signature/show.hpp>
#include <we/type/value/signature/name.hpp>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>

#include <iostream>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      as_signature::as_signature (const signature_type& signature)
        : _signature (signature)
      {}

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
              ( "<", "|", ">", boost::ref (l)
              , boost::bind (&visitor_show::print_value, this, _1)
              );
          }
          std::ostream&
          operator() (const std::map<value_type, value_type>& m) const
          {
            return print_container<std::map<value_type, value_type> >
              ( "<", "|", ">", boost::ref (m)
              , boost::bind (&visitor_show::print_map_item, this, _1)
              );
          }
          std::ostream& operator() (const std::vector<value_type>& v) const
          {
            return print_container<std::vector<value_type> >
              ( "<", "|", ">", boost::ref (v)
              , boost::bind (&visitor_show::print_value, this, _1)
              );
          }
          std::ostream& operator() (const std::set<value_type>& s) const
          {
            return print_container<std::set<value_type> >
              ( "<" , "|", ">", boost::ref (s)
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
          template<typename T>
          std::ostream& operator() (const T& x) const
          {
            return _os << name_of<T> (x);
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
            _os << x.first << " :: ";
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

      std::ostream& as_signature::operator() (std::ostream& os) const
      {
        os << "signature ";

        return boost::apply_visitor (visitor_show (os), _signature);
      }

      std::ostream& operator<< (std::ostream& os, const as_signature& as_sig)
      {
        return as_sig (os);
      }
    }
  }
}
