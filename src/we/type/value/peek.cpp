// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/peek.hpp>

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
        class visitor_peek : public boost::static_visitor<boost::optional<V&> >
        {
        public:
          visitor_peek ( const std::list<std::string>::const_iterator& key
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

            MIT field (m.begin());

            while (field != m.end())
            {
              if (field->first == *_key)
              {
                return boost::apply_visitor
                  ( visitor_peek<V,M,MIT> ( boost::next (_key)
                                          , _end
                                          , field->second
                                          )
                  , field->second
                  );
              }

              ++field;
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
      peek ( const std::list<std::string>::const_iterator& key
           , const std::list<std::string>::const_iterator& end
           , const value_type& node
           )
      {
        return boost::apply_visitor
          ( visitor_peek< const value_type
                        , const structured_type
                        , structured_type::const_iterator
                        > (key, end, node)
          , node
          );
      }
      boost::optional<const value_type&>
      peek (const std::list<std::string>& path, const value_type& node)
      {
        return peek (path.begin(), path.end(), node);
      }
      boost::optional<const value_type&>
      peek (const std::string& path, const value_type& node)
      {
        return peek (path::split (path), node);
      }

      boost::optional<value_type&>
      peek ( const std::list<std::string>::const_iterator& key
           , const std::list<std::string>::const_iterator& end
           , value_type& node
           )
      {
        return boost::apply_visitor
          ( visitor_peek< value_type
                        , structured_type
                        , structured_type::iterator
                        > (key, end, node)
          , node
          );
      }
      boost::optional<value_type&>
      peek (const std::list<std::string>& path, value_type& node)
      {
        return peek (path.begin(), path.end(), node);
      }
      boost::optional<value_type&>
      peek (const std::string& path, value_type& node)
      {
        return peek (path::split (path), node);
      }
    }
  }
}
