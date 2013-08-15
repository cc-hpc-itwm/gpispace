// mirko.rahn@itwm.fraunhofer.de

#include <we2/type/signature/resolve.hpp>
#include <we2/type/signature/is_literal.hpp>

#include <boost/foreach.hpp>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace
      {
        signature_type get ( const resolver_type& resolver
                           , const std::string& s
                           )
        {
          const boost::optional<signature_type> signature (resolver (s));

          if (!signature)
          {
            throw std::runtime_error ("Could not resolve " + s);
          }

          return *signature;
        }

        class resolve_structured : public boost::static_visitor<structured_type>
        {
        public:
          resolve_structured (const resolver_type&);
          structured_type operator()
            (const std::pair<std::string, structure_type>&) const;

        private:
          const resolver_type& _resolver;
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
            return s;
          }
        private:
          const std::string& _name;
        };

        class resolve_field : public boost::static_visitor<field_type>
        {
        public:
          resolve_field (const resolver_type& resolver)
            : _resolver (resolver)
          {}
          field_type operator()
            (const std::pair<std::string, std::string>& f) const
          {
            if (is_literal (f.second))
            {
              return field_type (f);
            }

            signature_type s (get (_resolver, f.second));

            return boost::apply_visitor (mk_field (f.first), s);
          }
          field_type operator () (const structured_type& s) const
          {
            return boost::apply_visitor (resolve_structured (_resolver), s);
          }

        private:
          const resolver_type& _resolver;
        };

        resolve_structured::resolve_structured (const resolver_type& resolver)
          : _resolver (resolver)
        {}
        structured_type resolve_structured::operator()
          (const std::pair<std::string, structure_type>& s) const
        {
          structure_type l;

          BOOST_FOREACH (const field_type& f, s.second)
          {
            l.push_back (boost::apply_visitor (resolve_field (_resolver), f));
          }

          return std::make_pair (s.first, l);
        }
      }

      signature_type resolve ( const structured_type& signature
                             , const resolver_type& resolver
                             )
      {
        return boost::apply_visitor (resolve_structured (resolver), signature);
      }
    }
  }
}
