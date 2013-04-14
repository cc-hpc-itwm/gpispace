// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/signature/show.hpp>
#include <we/type/value/signature/name_of.hpp>

#include <we/type/value/detail/show.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      as_signature::as_signature (const signature_type& signature)
        : _signature (signature)
      {}

      namespace
      {
        using show::parens_type;

        class parens_of : public boost::static_visitor<parens_type>
        {
        public:
          parens_type operator() (const std::list<value_type>&) const
          {
            return parens_type ("<", "|", ">");
          }
          parens_type operator() (const std::map<value_type, value_type>&) const
          {
            return parens_type ("<", "|", ">");
          }
          parens_type operator() (const std::vector<value_type>&) const
          {
            return parens_type ("<", "|", ">");
          }
          parens_type operator() (const std::set<value_type>&) const
          {
            return parens_type ("<", "|", ">");
          }
          parens_type operator() (const structured_type&) const
          {
            return parens_type ("[", ", ", "]");
          }
        };

        class show_literal : public boost::static_visitor<std::ostream&>
        {
        public:
          template<typename T>
          std::ostream& operator() (std::ostream& os, const T& x) const
          {
            return os << name_of<T> (x);
          }
        };
      }

      std::ostream& as_signature::operator() (std::ostream& os) const
      {
        os << "signature ";

        parens_of parens_of;
        show_literal show_literal;
        return boost::apply_visitor
               ( pnet::type::value::show::visitor_show
                 ( os
                 , boost::apply_visitor (show_literal)
                 , boost::apply_visitor (parens_of)
                 , " :: "
                 )
               , _signature
               );
      }

      std::ostream& operator<< (std::ostream& os, const as_signature& as_sig)
      {
        return as_sig (os);
      }
    }
  }
}
