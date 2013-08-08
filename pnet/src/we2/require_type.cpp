// mirko.rahn@itwm.fraunhofer.de

#include <we2/require_type.hpp>
#include <we2/exception.hpp>

#include <we2/type/value/name_of.hpp>
#include <we2/type/value/path/append.hpp>

#include <we2/type/signature/name.hpp>
#include <we2/type/signature/signature.hpp>

#include <we2/type/value/show.hpp>
#include <we2/type/signature/show.hpp>

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

    // ** RELAXED
    void require_type_relaxed ( std::list<std::string>&
                              , const type::value::value_type&
                              , const type::signature::signature_type&
                              );

    class eq_name
    {
    public:
      eq_name (const std::string& n) : _n (n) {}
      bool operator() (const type::signature::structure_type::value_type& x)
      {
        return _n == type::signature::name (x);
      }
    private:
      const std::string& _n;
    };
    class eq_first
    {
    public:
      eq_first (const std::string& n) : _n (n) {}
      template<typename T>
      bool operator() (const T& x)
      {
        return _n == x.first;
      }
    private:
      const std::string& _n;
    };

    class require_structured_relaxed : public boost::static_visitor<>
    {
    public:
      require_structured_relaxed ( std::list<std::string>& path
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
        {
          std::list<type::signature::structure_type::const_iterator> seen;

          for ( type::value::structured_type::const_iterator v_pos (_value.begin())
              ; v_pos != _value.end()
              ; ++v_pos
              )
          {
            type::signature::structure_type::const_iterator
              s_pos (s.second.begin());
            const type::signature::structure_type::const_iterator
              s_end (s.second.end());

            while (s_pos != s_end)
            {
              s_pos = std::find_if (s_pos, s_end, eq_name (v_pos->first));

              if (  s_pos != s_end
                 && std::find (seen.begin(), seen.end(), s_pos) == seen.end()
                 )
              {
                seen.push_back (s_pos);
                break;
              }
            }

            if (s_pos != s_end)
            {
              require_type_relaxed ( append (_path, v_pos->first)
                                   , v_pos->second
                                   , type::signature::signature (*s_pos)
                                   );
            }
            else
            {
              throw exception::unknown_field
                (v_pos->second, append (_path, v_pos->first));
            }
          }
        }

        {
          std::list<type::value::structured_type::const_iterator> seen;

          for ( type::signature::structure_type::const_iterator s_pos (s.second.begin())
              ; s_pos != s.second.end()
              ; ++s_pos
              )
          {
            type::value::structured_type::const_iterator v_pos (_value.begin());
            const type::value::structured_type::const_iterator v_end (_value.end());

            while (v_pos != v_end)
            {
              v_pos = std::find_if ( v_pos
                                   , v_end
                                   , eq_first (type::signature::name (*s_pos))
                                   );

              if (  v_pos != v_end
                 && std::find (seen.begin(), seen.end(), v_pos) == seen.end()
                 )
              {
                seen.push_back (v_pos);
                break;
              }
            }

            if (v_pos != v_end)
            {
              require_type_relaxed ( append (_path, v_pos->first)
                                   , v_pos->second
                                   , type::signature::signature (*s_pos)
                                   );
            }
            else
            {
              throw exception::missing_field
                (s, append (_path, type::signature::name (*s_pos)));
            }
          }
        }
      }

    private:
      std::list<std::string>& _path;
      const type::value::structured_type& _value;
    };

    class visitor_require_type_relaxed : public boost::static_visitor<>
    {
    public:
      visitor_require_type_relaxed (std::list<std::string>& path)
        : _path (path)
      {}

      void operator() ( const type::value::structured_type& v
                      , const type::signature::structured_type& s
                      ) const
      {
        boost::apply_visitor (require_structured_relaxed (_path, v), s);
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

    void require_type_relaxed ( std::list<std::string>& path
                              , const type::value::value_type& value
                              , const type::signature::signature_type& signature
                              )
    {
      boost::apply_visitor ( visitor_require_type_relaxed (path)
                           , value
                           , signature
                           );
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

  const type::value::value_type& require_type_relaxed
    ( const type::value::value_type& value
    , const type::signature::signature_type& signature
    , const std::string& field
    )
  {
    std::list<std::string> path;

    path.push_back (field);

    require_type_relaxed (path, value, signature);

    return value;
  }
}
