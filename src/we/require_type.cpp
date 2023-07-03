// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/exception.hpp>
#include <we/require_type.hpp>

#include <we/type/value/name_of.hpp>
#include <we/type/value/path/append.hpp>

#include <we/type/signature.hpp>

#include <algorithm>
#include <list>

namespace pnet
{
  namespace
  {
    class visitor_name : public ::boost::static_visitor<std::string const&>
    {
    public:
      std::string const& operator()
        (std::pair<std::string, std::string> const& f) const
      {
        return f.first;
      }
      std::string const& operator()
        (type::signature::structured_type const& s) const
      {
        return s.first;
      }
    };

    std::string const& name (type::signature::field_type const& s)
    {
      return ::boost::apply_visitor (visitor_name(), s);
    }

    class visitor_signature
      : public ::boost::static_visitor<type::signature::signature_type>
    {
    public:
      type::signature::signature_type operator()
        (std::pair<std::string, std::string> const& f) const
      {
        return f.second;
      }
      type::signature::signature_type operator()
        (type::signature::structured_type const& s) const
      {
        return s;
      }
    };

    type::signature::signature_type signature
      (type::signature::field_type const& s)
    {
      return ::boost::apply_visitor (visitor_signature(), s);
    }

    using type::value::path::append;

    void require_type ( std::list<std::string>&
                      , type::value::value_type const&
                      , type::signature::signature_type const&
                      );

    class require_structured : public ::boost::static_visitor<>
    {
    public:
      require_structured ( std::list<std::string>& path
                         , type::value::structured_type const& v
                         )
        : _path (path)
        , _value (v)
      {}

      void operator() (std::pair< std::string
                                      , type::signature::structure_type
                                      > const& s
                      ) const
      {
        auto v_pos (_value.begin());
        const type::value::structured_type::const_iterator v_end (_value.end());
        auto
          s_pos (s.second.begin());
        const type::signature::structure_type::const_iterator
          s_end (s.second.end());

        while (  v_pos != v_end
              && s_pos != s_end
              && v_pos->first == name (*s_pos)
              )
        {
          require_type ( append (_path, v_pos->first)
                       , v_pos->second
                       , signature (*s_pos)
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
            (s, _value, append (_path, name (*s_pos)));
        }
      }

    private:
      std::list<std::string>& _path;
      type::value::structured_type const& _value;
    };

    class visitor_require_type : public ::boost::static_visitor<>
    {
    public:
      visitor_require_type (std::list<std::string>& path)
        : _path (path)
      {}

      void operator() ( type::value::structured_type const& v
                      , type::signature::structured_type const& s
                      ) const
      {
        require_structured (_path, v) (s);
      }
      [[noreturn]] void operator()
        ( type::value::structured_type const& v
        , std::string const& s
        ) const
      {
        throw exception::type_mismatch (s, v, _path);
      }
      template<typename V>
        void operator() ( V const& v
                        , type::signature::structured_type const& s
                        ) const
      {
        throw exception::type_mismatch (s, v, _path);
      }
      template<typename V>
        void operator() ( V const& v
                        , std::string const& s
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
                      , type::value::value_type const& value
                      , type::signature::signature_type const& signature
                      )
   {
      ::boost::apply_visitor (visitor_require_type (path), value, signature);
    }
  }

  type::value::value_type const& require_type
    ( type::value::value_type const& value
    , type::signature::signature_type const& signature
    )
  {
    std::list<std::string> path;

    require_type (path, value, signature);

    return value;
  }

  type::value::value_type const& require_type
    ( type::value::value_type const& value
    , type::signature::signature_type const& signature
    , std::string const& field
    )
  {
    std::list<std::string> path;

    require_type (append (path, field), value, signature);

    return value;
  }
}
