// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_TYPE_VALUE_DETAIL_SHOW_HPP
#define PNET_SRC_WE_TYPE_VALUE_DETAIL_SHOW_HPP

#include <we/type/value/name_of.hpp>

#include <we/type/value/detail/literal.hpp>
#include <we/type/value/detail/container.hpp>

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
      namespace detail
      {
        class parens_type
        {
        public:
          parens_type ( const std::string& open
                      , const std::string& sep
                      , const std::string& close
                      )
            : _open (open)
            , _sep (sep)
            , _close (close)
          {}
          const std::string& open() const
          {
            return _open;
          }
          const std::string& sep() const
          {
            return _sep;
          }
          const std::string& close() const
          {
            return _close;
          }
        private:
          const std::string _open;
          const std::string _sep;
          const std::string _close;
        };

        typedef boost::function< parens_type (const value_container_type&)
                               > parens_of_type;

        typedef boost::function< std::ostream& ( boost::variant<std::ostream&>
                                               , const value_literal_type&
                                               )
                               > show_literal_type;

        class visitor_show : public boost::static_visitor<std::ostream&>
        {
        public:
          visitor_show ( std::ostream& os
                       , const show_literal_type& show_literal
                       , const parens_of_type& parens_of
                       , const std::string& sep_struct_item
                       )
            : _os (os)
            , _show_literal (show_literal)
            , _parens_of (parens_of)
            , _sep_struct_item (sep_struct_item)
          {}

          std::ostream& operator() (const std::list<value_type>& l) const
          {
            return print_container<std::list<value_type> >
              ( boost::ref (l)
              , boost::bind (&visitor_show::print_value, this, _1)
              );
          }
          std::ostream&
          operator() (const std::map<value_type, value_type>& m) const
          {
            return print_container<std::map<value_type, value_type> >
              ( boost::ref (m)
              , boost::bind (&visitor_show::print_map_item, this, _1)
              );
          }
          std::ostream& operator() (const std::set<value_type>& s) const
          {
            return print_container<std::set<value_type> >
              ( boost::ref (s)
              , boost::bind (&visitor_show::print_value, this, _1)
              );
          }
          std::ostream& operator() (const structured_type& m) const
          {
            return print_container<structured_type>
              ( boost::ref (m)
              , boost::bind (&visitor_show::print_struct_item, this, _1)
              );
          }
          template<typename T>
          std::ostream& operator() (const T& x) const
          {
            return _show_literal (_os, value_literal_type (x));
          }

        private:
          std::ostream& _os;
          const show_literal_type _show_literal;
          const parens_of_type _parens_of;
          const std::string _sep_struct_item;

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
            _os << x.first << _sep_struct_item;
            boost::apply_visitor (*this, x.second);
          }

          template<typename C>
          std::ostream& print_container
          ( const C& c
          , const boost::function<void (const typename C::value_type&)>& f
          ) const
          {
            const parens_type p (_parens_of (c));
            _os << name_of<C> (c) << " " << p.open();
            bool first (true);
            BOOST_FOREACH (const typename C::value_type& x, c)
            {
              if (!first)
              {
                _os << p.sep();
              }
              f (x);
              first = false;
            }
            return _os << p.close();
          }
        };
      }
    }
  }
}

#endif
