// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/get.hpp>

#include <we/type/value/path/split.hpp>

#include <boost/utility.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace
      {
        template<typename V, typename M, typename MIT>
        class visitor_get : public boost::static_visitor<boost::optional<V&> >
        {
        public:
          visitor_get ( const std::list<std::string>::const_iterator& key
                      , const std::list<std::string>::const_iterator& end
                      , V& node
                      )
            : _key (key)
            , _end (end)
            , _node (node)
          {}

          boost::optional<V&> operator() (M& m) const
          {
            if (_key == _end)
            {
              return _node;
            }

            MIT field (m.find (*_key));

            if (field != m.end())
              {
                return boost::apply_visitor
                  ( visitor_get<V,M,MIT> ( boost::next (_key)
                                         , _end
                                         , field->second
                                         )
                  , field->second
                  );
              }

            return boost::none;
          }

          template<typename T>
          boost::optional<V&> operator() (T&) const
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
          V& _node;
        };
      }

      boost::optional<const value_type&>
      get ( const std::list<std::string>::const_iterator& key
          , const std::list<std::string>::const_iterator& end
          , const value_type& node
          )
      {
        return boost::apply_visitor
          ( visitor_get< const value_type
                       , const structured_type
                       , const structured_type::const_iterator
                       > (key, end, node)
          , node
          );
      }
      boost::optional<const value_type&>
      get (const std::list<std::string>& path, const value_type& node)
      {
        return get (path.begin(), path.end(), node);
      }
      boost::optional<const value_type&>
      get (const std::string& path, const value_type& node)
      {
        return get (path::split (path), node);
      }

      boost::optional<value_type&>
      get_ref ( const std::list<std::string>::const_iterator& key
              , const std::list<std::string>::const_iterator& end
              , value_type& node
              )
      {
        return boost::apply_visitor
          ( visitor_get< value_type
                       , structured_type
                       , structured_type::iterator
                       > (key, end, node)
          , node
          );
      }
      boost::optional<value_type&>
      get_ref (const std::list<std::string>& path, value_type& node)
      {
        return get_ref (path.begin(), path.end(), node);
      }
      boost::optional<value_type&>
      get_ref (const std::string& path, value_type& node)
      {
        return get_ref (path::split (path), node);
      }
    }
  }
}
