// mirko.rahn@itwm.fraunhofer.de

#include <we2/type/value/poke.hpp>
#include <we2/type/value/path/split.hpp>

#include <boost/utility.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace
      {
        class visitor_poke : public boost::static_visitor<value_type&>
        {
        public:
          visitor_poke ( const std::list<std::string>::const_iterator& key
                       , const std::list<std::string>::const_iterator& end
                       , value_type& node
                       , const value_type& value
                       )
            : _key (key)
            , _end (end)
            , _node (node)
            , _value (value)
          {}

          value_type& operator() (structured_type& m) const
          {
            return (_key == _end) ? (_node = _value) : deeper (m);
          }

          template<typename T> value_type& operator() (T&) const
          {
            return (_key == _end)
              ? (_node = _value)
              : deeper (boost::get<structured_type&> (_node = structured_type())
                       )
              ;
          }

        private:
          const std::list<std::string>::const_iterator& _key;
          const std::list<std::string>::const_iterator& _end;
          value_type& _node;
          const value_type& _value;

          value_type& deeper (structured_type& m) const
          {
            structured_type::iterator pos (m.begin());

            while (pos != m.end())
            {
              if (pos->first == *_key)
              {
                pos = m.erase (pos);

                break;
              }

              ++pos;
            }

            pos = m.insert (pos, std::make_pair (*_key, value_type()));
            value_type& v (pos->second);

            return boost::apply_visitor
              (visitor_poke (boost::next (_key), _end, v, _value), v);
          }
        };
      }

      value_type& poke ( const std::list<std::string>::const_iterator& key
                       , const std::list<std::string>::const_iterator& end
                       , value_type& node
                       , const value_type& value
                       )
      {
        return boost::apply_visitor
          (visitor_poke (key, end, node, value), node);
      }
      value_type& poke ( const std::list<std::string>& path
                       , value_type& node
                       , const value_type& value
                       )
      {
        return poke (path.begin(), path.end(), node, value);
      }
      value_type& poke ( const std::string& path
                       , value_type& node
                       , const value_type& value
                       )
      {
        return poke (path::split (path), node, value);
      }
    }
  }
}
