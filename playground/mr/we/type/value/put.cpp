// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/put.hpp>

#include <boost/utility.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace
      {
        class visitor_put : public boost::static_visitor<value_type>
        {
        public:
          visitor_put ( const std::list<std::string>::const_iterator& key
                      , const std::list<std::string>::const_iterator& end
                      , const value_type& value
                      )
            : _key (key)
            , _end (end)
            , _value (value)
          {}

          value_type
          operator() (const std::map<std::string, value_type>& m) const
          {
            if (_key == _end)
              {
                return _value;
              }

            const std::map<std::string, value_type>::const_iterator field
              (m.find (*_key));

            std::map<std::string, value_type> m_copy (m);

            m_copy[*_key] = boost::apply_visitor
              ( visitor_put (boost::next (_key), _end, _value)
              , field == m.end() ? empty() : field->second
              );

            return value_type (m_copy);
          }

          template<typename T>
          value_type operator() (const T&) const
          {
            if (_key == _end)
              {
                return _value;
              }

            return operator() (std::map<std::string, value_type>());
          }

        private:
          const std::list<std::string>::const_iterator& _key;
          const std::list<std::string>::const_iterator& _end;
          const value_type& _value;
        };
      }

      value_type put ( const std::list<std::string>& keys
                     , const value_type& value
                     , const value_type& node
                     )
      {
        return boost::apply_visitor
          (visitor_put (keys.begin(), keys.end(), value), node);
      }
    }
  }
}
