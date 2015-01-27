// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <we/type/property.hpp>

#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/dump.hpp>

#include <fhg/util/boost/variant.hpp>
#include <fhg/util/xml.hpp>

#include <boost/optional.hpp>
#include <boost/utility.hpp>

#include <functional>
#include <iterator>

namespace we
{
  namespace type
  {
    namespace property
    {
      type::type ()
        : _value (pnet::type::value::structured_type())
      {}

      value_type const& type::value() const
      {
        return _value;
      }

      pnet::type::value::structured_type const& type::list() const
      {
        return boost::get<pnet::type::value::structured_type const&> (_value);
      }

      void type::set (const path_type& path, const value_type& val)
      {
        pnet::type::value::poke (path.begin(), path.end(), _value, val);
      }

      boost::optional<const value_type&>
        type::get (const path_type& path) const
      {
        return pnet::type::value::peek (path.begin(), path.end(), _value);
      }

      namespace
      {
        struct first_is : public std::unary_function<const std::string&, bool>
        {
          first_is (const std::string& what)
            : _what (what)
          {}

          template<typename T>
          bool operator() (const T& x)
          {
            return x.first == _what;
          }

        private:
          const std::string _what;
        };

        class visitor_remove : public boost::static_visitor<void>
        {
        public:
          visitor_remove ( const std::list<std::string>::const_iterator& key
                         , const std::list<std::string>::const_iterator& end
                         , value_type& node
                         )
            : _key (key)
            , _end (end)
            , _node (node)
          {}

          void operator() (pnet::type::value::structured_type& m) const
          {
            if (_key == _end)
            {
              _node = value_type();
            }
            else
            {
              pnet::type::value::structured_type::iterator pos
                (std::find_if (m.begin(), m.end(), first_is (*_key)));

              if (pos == m.end())
              {
                throw std::runtime_error ("value_type::remove: key not found");
              }

              const std::list<std::string>::const_iterator next
                (std::next (_key));

              if (next == _end)
              {
                m.erase (pos);
              }
              else
              {
                value_type& v (pos->second);

                boost::apply_visitor (visitor_remove (next, _end, v), v);
              }
            }
          }

          template<typename T> void operator() (T&) const
          {
            if (_key == _end)
            {
              _node = value_type();
            }
            else
            {
              throw std::runtime_error
                ("value_type::remove: trying to remove from unstructured value");
            }
          }

        private:
          const std::list<std::string>::const_iterator& _key;
          const std::list<std::string>::const_iterator& _end;
          value_type& _node;
        };
      }

      void type::del (path_type const& path)
      {
        boost::apply_visitor (visitor_remove (path.begin(), path.end(), _value), _value);
      }

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream& s, const type& p)
        {
          pnet::type::value::dump (s, p.value());
        }
      }

      std::ostream& operator << (std::ostream& s, const type& t)
      {
        ::fhg::util::xml::xmlstream xs (s);

        dump::dump (xs, t);

        return s;
      }
    }
  }
}
