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
          visitor_poke ( const std::list<std::string>::const_iterator& pos_key
                       , const std::list<std::string>::const_iterator& end_key
                       , value_type& node
                       , const value_type& value
                       )
            : _pos_key (pos_key)
            , _end_key (end_key)
            , _node (node)
            , _value (value)
          {}

          value_type& operator() (std::map<std::string, value_type>& m) const
          {
            return (_pos_key == _end_key) ? (_node = _value) : deeper (m);
          }

          template<typename T> value_type& operator() (T&) const
          {
            if (_pos_key == _end_key)
              {
                return _node = _value;
              }

            return deeper  (boost::get<std::map<std::string,value_type>&>
                            (_node = empty())
                          );
          }

        private:
          const std::list<std::string>::const_iterator& _pos_key;
          const std::list<std::string>::const_iterator& _end_key;
          value_type& _node;
          const value_type& _value;

          value_type& deeper (std::map<std::string, value_type>& m) const
          {
            value_type& v (m[*_pos_key]);

            return boost::apply_visitor
              (visitor_poke (boost::next (_pos_key), _end_key, v, _value), v);
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
