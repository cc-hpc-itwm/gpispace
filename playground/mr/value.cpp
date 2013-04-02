// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnet_type_value
#include <boost/test/unit_test.hpp>

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/utility.hpp>
#include <boost/variant.hpp>

#include <list>
#include <map>
#include <set>
#include <string>

#include <iostream>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      typedef boost::make_recursive_variant
              < int
              , long
              , std::string
              , std::list<boost::recursive_variant_>
              , std::map<boost::recursive_variant_, boost::recursive_variant_>
              , std::set<boost::recursive_variant_>
              , std::map<std::string, boost::recursive_variant_>
              >::type value_type;

      value_type empty()
      {
        return (std::map<std::string, value_type>());
      }

      namespace visitor
      {
        class show : public boost::static_visitor<std::ostream&>
        {
        public:
          show (std::ostream& os) : _os (os) {}
          std::ostream& operator() (const int& i) const
          {
            return _os << i;
          }
          std::ostream& operator() (const long& i) const
          {
            return _os << i << "L";
          }
          std::ostream& operator() (const std::string& s) const
          {
            return _os << "\"" << s << "\"";
          }
          std::ostream& operator() (const std::list<value_type>& l) const
          {
            _os << "list (";
            bool first (true);
            BOOST_FOREACH (const value_type& v, l)
              {
                if (!first)
                  {
                    _os << ", ";
                  }
                boost::apply_visitor (*this, v);
                first = false;
              }
            return _os << ")";
          }
          std::ostream&
          operator() (const std::map<value_type, value_type>& m) const
          {
            _os << "map [";
            bool first (true);
            typedef std::pair<value_type, value_type> kv_type;
            BOOST_FOREACH (const kv_type& kv, m)
              {
                if (!first)
                  {
                    _os << ", ";
                  }
                boost::apply_visitor (*this, kv.first);
                _os << " => ";
                boost::apply_visitor (*this, kv.second);
                first = false;
              }
            return _os << "]";
          }
          std::ostream& operator() (const std::set<value_type>& s) const
          {
            _os << "set {";
            bool first (true);
            BOOST_FOREACH (const value_type& v, s)
              {
                if (!first)
                  {
                    _os << ", ";
                  }
                boost::apply_visitor (*this, v);
                first = false;
              }
            return _os << "}";
          }
          std::ostream& operator() (const std::map<std::string, value_type>& m) const
          {
            _os << "struct [";
            bool first (true);
            typedef std::pair<std::string, value_type> kv_type;
            BOOST_FOREACH (const kv_type& kv, m)
              {
                if (!first)
                  {
                    _os << ", ";
                  }
                _os << kv.first << " := ";
                boost::apply_visitor (*this, kv.second);
                first = false;
              }
            return _os << "]";
          }
        private:
          std::ostream& _os;
        };
      }

      namespace visitor
      {
        class get : public boost::static_visitor<boost::optional<value_type> >
        {
        public:
          get ( const std::list<std::string>::const_iterator& pos_key
              , const std::list<std::string>::const_iterator& end_key
              )
            : _pos_key (pos_key)
            , _end_key (end_key)
          {}

          boost::optional<value_type>
          operator() (const std::map<std::string, value_type>& m) const
          {
            if (_pos_key == _end_key)
              {
                return value_type (m);
              }

            const std::map<std::string, value_type>::const_iterator pos
              (m.find (*_pos_key));

            if (pos != m.end())
              {
                return boost::apply_visitor
                  (get (boost::next (_pos_key), _end_key), pos->second);
              }

            return boost::none;
          }

          template<typename T>
          boost::optional<value_type> operator() (const T& x) const
          {
            if (_pos_key == _end_key)
              {
                return value_type (x);
              }

            return boost::none;
          }

        private:
          const std::list<std::string>::const_iterator& _pos_key;
          const std::list<std::string>::const_iterator& _end_key;
        };
      }

      //! \todo Avoid the copy, get a ref instead!?
      boost::optional<value_type> get ( const std::list<std::string>& keys
                                      , const value_type& v
                                      )
      {
        return boost::apply_visitor
          (visitor::get (keys.begin(), keys.end()), v);
      }

      namespace visitor
      {
        class put : public boost::static_visitor<value_type>
        {
        public:
          put ( const std::list<std::string>::const_iterator& pos_key
              , const std::list<std::string>::const_iterator& end_key
              , const value_type& x
              )
            : _pos_key (pos_key)
            , _end_key (end_key)
            , _x (x)
          {}

          value_type
          operator() (const std::map<std::string, value_type>& m) const
          {
            if (_pos_key == _end_key)
              {
                return _x;
              }

            const std::map<std::string, value_type>::const_iterator pos
              (m.find (*_pos_key));

            std::map<std::string, value_type> m_copy (m);

            m_copy[*_pos_key] = boost::apply_visitor
              ( put (boost::next (_pos_key), _end_key, _x)
              , pos == m.end() ? empty() : pos->second
              );

            return value_type (m_copy);
          }

          template<typename T>
          value_type operator() (const T&) const
          {
            if (_pos_key == _end_key)
              {
                return _x;
              }

            return this->operator() (std::map<std::string, value_type>());
          }

        private:
          const std::list<std::string>::const_iterator& _pos_key;
          const std::list<std::string>::const_iterator& _end_key;
          const value_type& _x;
        };
      }

      value_type put ( const std::list<std::string>& keys
                     , const value_type& x
                     , const value_type& v
                     )
      {
        return boost::apply_visitor
          (visitor::put (keys.begin(), keys.end(), x), v);
      }
    }
  }
}

std::ostream&
operator<< (std::ostream& os, const pnet::type::value::value_type& v)
{
  return boost::apply_visitor (pnet::type::value::visitor::show (os), v);
}

BOOST_AUTO_TEST_CASE (get)
{
  using pnet::type::value::value_type;
  using pnet::type::value::get;

  const value_type s (std::string ("s"));

  BOOST_REQUIRE_EQUAL (boost::get<std::string> (s), "s");
  BOOST_REQUIRE_EQUAL (boost::get<const std::string&> (s), "s");
  BOOST_REQUIRE_THROW (boost::get<std::string&> (s), boost::bad_get);

  value_type i (int (0));

  BOOST_REQUIRE_EQUAL (boost::get<const int&> (i), 0);

  boost::get<int&> (i) = 1;

  BOOST_REQUIRE_EQUAL (boost::get<const int&> (i), 1);

  value_type l1;
  {
    std::list<value_type> _l1;
    _l1.push_back (i);
    _l1.push_back (s);
    _l1.push_back (3L);
    l1 = _l1;
  }

  value_type m1 (pnet::type::value::empty());
  boost::get<std::map<std::string, value_type>&> (m1)["i"] = i;
  boost::get<std::map<std::string, value_type>&> (m1)["s"] = s;
  boost::get<std::map<std::string, value_type>&> (m1)["l1"] = l1;

  std::list<value_type> _l2;
  _l2.push_back (l1);
  _l2.push_back (m1);
  const value_type l2 (_l2);

  std::map<value_type, value_type> _mv;
  _mv[i] = s;
  _mv[m1] = l2;
  const value_type mv (_mv);

  std::set<value_type> _set;
  _set.insert (i);
  _set.insert (i);
  _set.insert (s);
  _set.insert (l1);
  const value_type set (_set);

  std::map<std::string, value_type> _m2;
  _m2["m1"] = m1;
  _m2["l2"] = l2;
  _m2["mv"] = mv;
  _m2["set"] = set;
  const value_type m2 (_m2);

  std::cout << m2 << std::endl;

  {
    std::list<std::string> keys;
    keys.push_back ("set");
    BOOST_CHECK (get (keys, m2));
    BOOST_CHECK (*get (keys, m2) == set);
    // BOOST_REQUIRE_EQUAL (*get (keys, m2), set);
    // | Does not compile. Why?
  }
  {
    std::list<std::string> keys;
    keys.push_back ("m1");
    keys.push_back ("l1");
    BOOST_CHECK (get (keys, m2));
    BOOST_CHECK (*get (keys, m2) == l1);
  }
}

BOOST_AUTO_TEST_CASE (put_get)
{
  using pnet::type::value::value_type;
  using pnet::type::value::get;
  using pnet::type::value::put;

  const value_type i (int (1));
  const value_type s (std::string ("s"));

  std::list<std::string> keys1;
  keys1.push_back ("key1");
  const value_type v1 (put (keys1, i, pnet::type::value::empty()));
  BOOST_CHECK (get (keys1, v1));
  BOOST_CHECK (*get (keys1, v1) == i);

  std::list<std::string> keys2;
  keys2.push_back ("key2");
  const value_type v2 (put (keys2, s, v1));

  BOOST_CHECK (get (keys1, v2));
  BOOST_CHECK (*get (keys1, v2) == i);
  BOOST_CHECK (get (keys2, v2));
  BOOST_CHECK (*get (keys2, v2) == s);

  std::list<std::string> keysm;
  keysm.push_back ("m");
  const value_type m (put (keysm, v2, pnet::type::value::empty()));

  keysm.push_back ("key1");
  BOOST_CHECK (get (keysm, m));
  BOOST_CHECK (*get (keysm, m) == i);
  keysm.pop_back();

  keysm.push_back ("key2");
  BOOST_CHECK (get (keysm, m));
  BOOST_CHECK (*get (keysm, m) == s);
}
