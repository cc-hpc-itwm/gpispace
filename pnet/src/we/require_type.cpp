// mirko.rahn@itwm.fraunhofer.de

#include <we/require_type.hpp>
#include <we/exception.hpp>

#include <we/type/value/name_of.hpp>
#include <we/type/value/path/append.hpp>

#include <we/type/signature/name.hpp>
#include <we/type/signature/signature.hpp>

#include <we/type/value/show.hpp>
#include <we/type/signature/show.hpp>

#include <algorithm>
#include <list>

namespace pnet
{
  namespace
  {
    using type::value::path::append;

    void require_type ( std::list<std::string>&
                      , const type::value::value_type&
                      , const type::signature::signature_type&
                      );

    class require_structured : public boost::static_visitor<>
    {
    public:
      require_structured ( std::list<std::string>& path
                         , const type::value::structured_type& v
                         )
        : _path (path)
        , _value (v)
      {}

      void operator() (const std::pair< std::string
                                      , type::signature::structure_type
                                      >& s
                      ) const
      {
        type::value::structured_type::const_iterator v_pos (_value.begin());
        const type::value::structured_type::const_iterator v_end (_value.end());
        type::signature::structure_type::const_iterator
          s_pos (s.second.begin());
        const type::signature::structure_type::const_iterator
          s_end (s.second.end());

        while (  v_pos != v_end
              && s_pos != s_end
              && v_pos->first == type::signature::name (*s_pos)
              )
        {
          require_type ( append (_path, v_pos->first)
                       , v_pos->second
                       , type::signature::signature (*s_pos)
                       );

          ++v_pos;
          ++s_pos;
        }

        if (v_pos != v_end)
        {
          throw exception::unknown_field
            (v_pos->second, append (_path, v_pos->first));
        }

        if (s_pos != s_end)
        {
          throw exception::missing_field
            (s, append (_path, type::signature::name (*s_pos)));
        }
      }

    private:
      std::list<std::string>& _path;
      const type::value::structured_type& _value;
    };

    class visitor_require_type : public boost::static_visitor<>
    {
    public:
      visitor_require_type (std::list<std::string>& path)
        : _path (path)
      {}

      void operator() ( const type::value::structured_type& v
                      , const type::signature::structured_type& s
                      ) const
      {
        boost::apply_visitor (require_structured (_path, v), s);
      }
      void operator() ( const type::value::structured_type& v
                      , const std::string& s
                      ) const
      {
        throw exception::type_mismatch (s, v, _path);
      }
      template<typename V>
        void operator() ( const V& v
                        , const type::signature::structured_type& s
                        ) const
      {
        throw exception::type_mismatch (s, v, _path);
      }
      template<typename V>
        void operator() ( const V& v
                        , const std::string& s
                        ) const
      {
        if (type::value::name_of (v) != s)
        {
          throw exception::type_mismatch (s, v, _path);
        }
      }

    private:
      std::list<std::string>& _path;
    };

    void require_type ( std::list<std::string>& path
                      , const type::value::value_type& value
                      , const type::signature::signature_type& signature
                      )
   {
      boost::apply_visitor (visitor_require_type (path), value, signature);
    }
  }

  const type::value::value_type& require_type
    ( const type::value::value_type& value
    , const type::signature::signature_type& signature
    )
  {
    std::list<std::string> path;

    require_type (path, value, signature);

    return value;
  }

  const type::value::value_type& require_type
    ( const type::value::value_type& value
    , const type::signature::signature_type& signature
    , const std::string& field
    )
  {
    std::list<std::string> path;

    path.push_back (field);

    require_type (path, value, signature);

    return value;
  }
}
