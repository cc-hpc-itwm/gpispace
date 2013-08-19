// mirko.rahn@itwm.fraunhofer.de

#include <we/type/signature/resolve.hpp>
#include <we/type/signature/is_literal.hpp>
#include <we/type/value/path/append.hpp>

#include <we/exception.hpp>

#include <boost/foreach.hpp>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace
      {
        using type::value::path::append;

        signature_type get ( const resolver_type& resolver
                           , const std::list<std::string>& path
                           , const std::string& s
                           )
        {
          const boost::optional<signature_type> signature (resolver (s));

          if (!signature)
          {
            throw exception::could_not_resolve (s, path);
          }

          return *signature;
        }

        class resolve_structured : public boost::static_visitor<structured_type>
        {
        public:
          resolve_structured (const resolver_type&, std::list<std::string>&);
          structured_type operator()
            (const std::pair<std::string, structure_type>&) const;

        private:
          const resolver_type& _resolver;
          std::list<std::string>& _path;
        };

        class with_name : public boost::static_visitor<field_type>
        {
        public:
          with_name (const std::string& name)
            : _name (name)
          {}
          field_type operator()
            (const std::pair<std::string, structure_type>& s) const
          {
            return structured_type (std::make_pair (_name, s.second));
          }

        private:
          const std::string& _name;
        };

        class mk_field : public boost::static_visitor<field_type>
        {
        public:
          mk_field (const std::string& name)
            : _name (name)
          {}
          field_type operator() (const std::string& t) const
          {
            return std::make_pair (_name, t);
          }
          field_type operator() (const structured_type& s) const
          {
            return boost::apply_visitor (with_name (_name), s);
          }
        private:
          const std::string& _name;
        };

        class resolve_field : public boost::static_visitor<field_type>
        {
        public:
          resolve_field ( const resolver_type& resolver
                        , std::list<std::string>& path
                        )
            : _resolver (resolver)
            , _path (path)
          {}
          field_type operator()
            (const std::pair<std::string, std::string>& f) const
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

            return boost::apply_visitor (mk_field (f.first), s);
          }
          field_type operator () (const structured_type& s) const
          {
            return boost::apply_visitor ( resolve_structured (_resolver, _path)
                                        , s
                                        );
          }

        private:
          const resolver_type& _resolver;
          std::list<std::string>& _path;
        };

        resolve_structured::resolve_structured
          ( const resolver_type& resolver
          , std::list<std::string>& path
          )
          : _resolver (resolver)
          , _path (path)
        {}
        structured_type resolve_structured::operator()
          (const std::pair<std::string, structure_type>& s) const
        {
          structure_type l;

          BOOST_FOREACH (const field_type& f, s.second)
          {
            l.push_back
              (boost::apply_visitor (resolve_field ( _resolver
                                                   , append (_path, s.first)
                                                   )
                                    , f
                                    )
              );
          }

          return std::make_pair (s.first, l);
        }
      }

      signature_type resolve ( const structured_type& signature
                             , const resolver_type& resolver
                             )
      {
        std::list<std::string> path;

        return boost::apply_visitor ( resolve_structured (resolver, path)
                                    , signature
                                    );
      }
    }
  }
}
