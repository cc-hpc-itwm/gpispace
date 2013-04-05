// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/poke.hpp>

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
              : deeper (boost::get<structured_type&> (_node = empty()))
              ;
          }

        private:
          const std::list<std::string>::const_iterator& _key;
          const std::list<std::string>::const_iterator& _end;
          value_type& _node;
          const value_type& _value;

          value_type& deeper (structured_type& m) const
          {
            value_type& v (m[*_key]);

            return boost::apply_visitor
              (visitor_poke (boost::next (_key), _end, v, _value), v);
          }
        };
      }

      value_type& poke ( const std::list<std::string>& path
                       , value_type& node
                       , const value_type& value
                       )
      {
        return boost::apply_visitor
          (visitor_poke (path.begin(), path.end(), node, value), node);
      }
    }
  }
}
