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
          visitor_put ( const std::list<std::string>::const_iterator& pos_key
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
              ( visitor_put (boost::next (_pos_key), _end_key, _x)
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
          (visitor_put (keys.begin(), keys.end(), x), v);
      }
    }
  }
}
