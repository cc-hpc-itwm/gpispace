// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/get.hpp>

#include <boost/utility.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace
      {
        class visitor_get
          : public boost::static_visitor<boost::optional<value_type> >
        {
        public:
          visitor_get ( const std::list<std::string>::const_iterator& pos_key
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
                  (visitor_get (boost::next (_pos_key), _end_key), pos->second);
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

      boost::optional<value_type> get ( const std::list<std::string>& keys
                                      , const value_type& v
                                      )
      {
        return boost::apply_visitor
          (visitor_get (keys.begin(), keys.end()), v);
      }
    }
  }
}
