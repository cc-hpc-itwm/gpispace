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
          visitor_get ( const std::list<std::string>::const_iterator& key
                      , const std::list<std::string>::const_iterator& end
                      , const value_type& node
                      )
            : _key (key)
            , _end (end)
            , _node (node)
          {}

          boost::optional<const value_type&>
          operator() (const std::map<std::string, value_type>& m) const
          {
            if (_key == _end)
            {
              return _node;
            }

            const std::map<std::string, value_type>::const_iterator field
              (m.find (*_key));

            if (field != m.end())
              {
                return boost::apply_visitor
                  ( visitor_get ( boost::next (_key)
                                , _end
                                , field->second
                                )
                  , field->second
                  );
              }

            return boost::none;
          }

          template<typename T>
          boost::optional<const value_type&> operator() (const T&) const
          {
            if (_key == _end)
            {
              return _node;
            }

            return boost::none;
          }

        private:
          const std::list<std::string>::const_iterator& _key;
          const std::list<std::string>::const_iterator& _end;
          const value_type& _node;
        };

        class visitor_get_ref
          : public boost::static_visitor<boost::optional<value_type&> >
        {
        public:
          visitor_get_ref
            ( const std::list<std::string>::const_iterator& pos
            , const std::list<std::string>::const_iterator& end
            , value_type& value
            )
              : _key (pos)
              , _end (end)
              , _node (value)
          {}

          boost::optional<value_type&>
          operator() (std::map<std::string, value_type>& m) const
          {
            if (_key == _end)
            {
              return _node;
            }

            const std::map<std::string, value_type>::iterator pos
              (m.find (*_key));

            if (pos != m.end())
              {
                return boost::apply_visitor
                  ( visitor_get_ref ( boost::next (_key)
                                    , _end
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
            if (_key == _end)
            {
              return _node;
            }

            return boost::none;
          }

        private:
          const std::list<std::string>::const_iterator& _key;
          const std::list<std::string>::const_iterator& _end;
          value_type& _node;
        };
      }

      boost::optional<const value_type&>
      get ( const std::list<std::string>& path
          , const value_type& node
          )
      {
        return boost::apply_visitor
          (visitor_get (path.begin(), path.end(), node), node);
      }
      boost::optional<value_type&>
      get_ref ( const std::list<std::string>& path
              , value_type& node
              )
      {
        return boost::apply_visitor
          (visitor_get_ref (path.begin(), path.end(), node), node);
      }
    }
  }
}
