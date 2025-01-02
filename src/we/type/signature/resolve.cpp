// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/type/signature/is_literal.hpp>
#include <we/type/signature/resolve.hpp>
#include <we/type/value/path/append.hpp>

#include <we/exception.hpp>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace
      {
        using type::value::path::append;

        signature_type get ( resolver_type const& resolver
                           , std::list<std::string> const& path
                           , std::string const& s
                           )
        {
          const ::boost::optional<signature_type> signature (resolver (s));

          if (!signature)
          {
            throw exception::could_not_resolve (s, path);
          }

          return *signature;
        }

        class resolve_structured : public ::boost::static_visitor<structured_type>
        {
        public:
          resolve_structured (resolver_type const&, std::list<std::string>&);
          structured_type operator() (structured_type const&) const;

        private:
          resolver_type const& _resolver;
          std::list<std::string>& _path;
        };

        class mk_field : public ::boost::static_visitor<field_type>
        {
        public:
          mk_field (std::string const& name)
            : _name (name)
          {}
          field_type operator() (std::string const& t) const
          {
            return std::make_pair (_name, t);
          }
          field_type operator() (structured_type const& s) const
          {
            return structured_type (std::make_pair (_name, s.second));
          }
        private:
          std::string const& _name;
        };

        class resolve_field : public ::boost::static_visitor<field_type>
        {
        public:
          resolve_field ( resolver_type const& resolver
                        , std::list<std::string>& path
                        )
            : _resolver (resolver)
            , _path (path)
          {}
          field_type operator()
            (std::pair<std::string, std::string> const& f) const
          {
            if (is_literal (f.second))
            {
              return field_type (f);
            }

            const signature_type s (get ( _resolver
                                        , append (_path, f.first)
                                        , f.second
                                        )
                                   );

            return ::boost::apply_visitor (mk_field (f.first), s);
          }
          field_type operator () (structured_type const& s) const
          {
            return resolve_structured (_resolver, _path) (s);
          }

        private:
          resolver_type const& _resolver;
          std::list<std::string>& _path;
        };

        resolve_structured::resolve_structured
          ( resolver_type const& resolver
          , std::list<std::string>& path
          )
          : _resolver (resolver)
          , _path (path)
        {}
        structured_type resolve_structured::operator()
          (structured_type const& s) const
        {
          structure_type l;

          for (field_type const& f : s.second)
          {
            l.push_back
              (::boost::apply_visitor (resolve_field ( _resolver
                                                   , append (_path, s.first)
                                                   )
                                    , f
                                    )
              );
          }

          return std::make_pair (s.first, l);
        }
      }

      signature_type resolve ( structured_type const& signature
                             , resolver_type const& resolver
                             )
      {
        std::list<std::string> path;

        return resolve_structured (resolver, path) (signature);
      }
    }
  }
}
