// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/remove.hpp>
#include <we/type/value/path/split.hpp>

#include <boost/utility.hpp>

#include <functional>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace
      {
        struct first_is : public std::unary_function<const std::string&, bool>
        {
          first_is (const std::string& what)
            : _what (what)
          {}

          template<typename T>
          bool operator() (const T& x)
          {
            return x.first == _what;
          }

        private:
          const std::string _what;
        };

        class visitor_remove : public boost::static_visitor<void>
        {
        public:
          visitor_remove ( const std::list<std::string>::const_iterator& key
                         , const std::list<std::string>::const_iterator& end
                         , value_type& node
                         )
            : _key (key)
            , _end (end)
            , _node (node)
          {}

          void operator() (structured_type& m) const
          {
            if (_key == _end)
            {
              _node = value_type();
            }
            else
            {
              structured_type::iterator pos
                (std::find_if (m.begin(), m.end(), first_is (*_key)));

              if (pos == m.end())
              {
                throw std::runtime_error ("value_type::remove: key not found");
              }

              const std::list<std::string>::const_iterator next
                (boost::next (_key));

              if (next == _end)
              {
                m.erase (pos);
              }
              else
              {
                value_type& v (pos->second);

                boost::apply_visitor (visitor_remove (next, _end, v), v);
              }
            }
          }

          template<typename T> void operator() (T&) const
          {
            if (_key == _end)
            {
              _node = value_type();
            }
            else
            {
              throw std::runtime_error
                ("value_type::remove: trying to remove from unstructured value");
            }
          }

        private:
          const std::list<std::string>::const_iterator& _key;
          const std::list<std::string>::const_iterator& _end;
          value_type& _node;
        };
      }

      void remove ( const std::list<std::string>::const_iterator& key
                  , const std::list<std::string>::const_iterator& end
                  , value_type& node
                  )
      {
        boost::apply_visitor (visitor_remove (key, end, node), node);
      }
      void remove ( const std::list<std::string>& path
                  , value_type& node
                  )
      {
        remove (path.begin(), path.end(), node);
      }
      void remove ( const std::string& path
                  , value_type& node
                  )
      {
        remove (path::split (path), node);
      }
    }
  }
}
