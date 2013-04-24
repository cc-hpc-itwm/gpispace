// mirko.rahn@itwm.fraunhofer.de

#include <we/type/signature/show.hpp>

#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <iostream>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace
      {
        class visitor_show : public boost::static_visitor<std::ostream&>
        {
        public:
          visitor_show (std::ostream& os)
            : _os (os)
          {}

          std::ostream& operator() (const std::string& name) const
          {
            return _os << name;
          }
          std::ostream& operator() (const std::list<signature_type>& l) const
          {
            return print_container<std::list<signature_type> >
              ( std::string ("list"), boost::ref (l)
              , boost::bind (&visitor_show::print_signature, this, _1)
              );
          }
          std::ostream& operator() (const std::set<signature_type>& s) const
          {
            return print_container<std::set<signature_type> >
              ( std::string ("set"), boost::ref (s)
              , boost::bind (&visitor_show::print_signature, this, _1)
              );
          }
          std::ostream&
          operator() (const std::map<signature_type, signature_type>& m) const
          {
            return print_container<std::map<signature_type, signature_type> >
              ( std::string ("map"), boost::ref (m)
              , boost::bind (&visitor_show::print_map_item, this, _1)
              );
          }
          std::ostream&
          operator() (const field_type& field) const
          {
            _os << field.first << " => [";
            BOOST_FOREACH (const structured_type::value_type& x, field.second)
            {
              _os << x.first << " :: ";
              boost::apply_visitor (*this, x.second);
            }
            return _os << "]";
          }
        private:
          std::ostream& _os;

          void print_signature (const signature_type& sig) const
          {
            boost::apply_visitor (*this, sig);
          }
          void print_map_item (const std::pair< signature_type
                                              , signature_type
                                              >& p) const
          {
            boost::apply_visitor (*this, p.first);
            _os << "->";
            boost::apply_visitor (*this, p.second);
          }

          template<typename C>
          std::ostream& print_container
          ( const std::string& name
          , const C& c
          , const boost::function<void (const typename C::value_type&)>& f
          ) const
          {
            _os << name << "{";
            bool first (true);
            BOOST_FOREACH (const typename C::value_type& x, c)
            {
              if (!first)
              {
                _os << "|";
              }
              f (x);
              first = false;
            }
            return _os << "}";
          }
        };
      }

      show::show (const signature_type& signature)
        : _signature (signature)
      {}

      std::ostream& show::operator() (std::ostream& os) const
      {
        return boost::apply_visitor (visitor_show (os), _signature);
      }

      std::ostream& operator<< (std::ostream& os, const show& s)
      {
        return s (os);
      }
    }
  }
}
