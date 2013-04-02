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
          : public boost::static_visitor<boost::optional<const value_type&> >
        {
        public:
          visitor_get ( const std::list<std::string>::const_iterator& pos_key
                      , const std::list<std::string>::const_iterator& end_key
                      , const value_type& value
                      )
            : _pos_key (pos_key)
            , _end_key (end_key)
            , _value (value)
          {}

          boost::optional<const value_type&>
          operator() (const std::map<std::string, value_type>& m) const
          {
            if (_pos_key == _end_key)
            {
              return _value;
            }

            const std::map<std::string, value_type>::const_iterator pos
              (m.find (*_pos_key));

            if (pos != m.end())
              {
                return boost::apply_visitor
                  ( visitor_get ( boost::next (_pos_key)
                                , _end_key
                                , pos->second
                                )
                  , pos->second
                  );
              }

            return boost::none;
          }

          template<typename T>
          boost::optional<const value_type&> operator() (const T&) const
          {
            if (_pos_key == _end_key)
            {
              return _value;
            }

            return boost::none;
          }

        private:
          const std::list<std::string>::const_iterator& _pos_key;
          const std::list<std::string>::const_iterator& _end_key;
          const value_type& _value;
        };

        class visitor_get_ref
          : public boost::static_visitor<boost::optional<value_type&> >
        {
        public:
          visitor_get_ref
            ( const std::list<std::string>::const_iterator& pos_key
            , const std::list<std::string>::const_iterator& end_key
            , value_type& value
            )
              : _pos_key (pos_key)
              , _end_key (end_key)
              , _value (value)
          {}

          boost::optional<value_type&>
          operator() (std::map<std::string, value_type>& m) const
          {
            if (_pos_key == _end_key)
            {
              return _value;
            }

            std::map<std::string, value_type>::iterator pos
              (m.find (*_pos_key));

            if (pos != m.end())
              {
                return boost::apply_visitor
                  ( visitor_get_ref ( boost::next (_pos_key)
                                    , _end_key
                                    , pos->second
                                    )
                  , pos->second
                  );
              }

            return boost::none;
          }

          template<typename T>
          boost::optional<value_type&> operator() (T&) const
          {
            if (_pos_key == _end_key)
            {
              return _value;
            }

            return boost::none;
          }

        private:
          const std::list<std::string>::const_iterator& _pos_key;
          const std::list<std::string>::const_iterator& _end_key;
          value_type& _value;
        };
      }

      boost::optional<const value_type&>
      get ( const std::list<std::string>& keys
          , const value_type& v
          )
      {
        return boost::apply_visitor
          (visitor_get (keys.begin(), keys.end(), v), v);
      }
      boost::optional<value_type&>
      get_ref ( const std::list<std::string>& keys
              , value_type& v
              )
      {
        return boost::apply_visitor
          (visitor_get_ref (keys.begin(), keys.end(), v), v);
      }
    }
  }
}
