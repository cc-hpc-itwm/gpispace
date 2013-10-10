// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/type/calculate.hpp>
#include <we/expr/token/type.hpp>

#include <we/type/signature/show.hpp>

#include <we/signature_of.hpp>
#include <we/exception.hpp>

#include <boost/format.hpp>

namespace pnet
{
  namespace expr
  {
    namespace type
    {
      namespace
      {
        class visitor_calculate
          : public boost::static_visitor<pnet::type::signature::signature_type>
        {
        public:
          visitor_calculate
          (const pnet::type::signature::resolver_type& resolve)
            : _resolve (resolve)
          {}

          pnet::type::signature::signature_type
          operator() (const pnet::type::value::value_type& v) const
          {
            return pnet::signature_of (v);
          }
          pnet::type::signature::signature_type
          operator() (const std::list<std::string>& path) const
          {
            return std::string ("void");
          }
          pnet::type::signature::signature_type
          operator() (const ::expr::parse::node::unary_t& u) const
          {
            pnet::type::signature::signature_type s0
              (boost::apply_visitor (*this, u.child));

            return std::string ("void");
          }
          pnet::type::signature::signature_type
          operator() (const ::expr::parse::node::binary_t& b) const
          {
            pnet::type::signature::signature_type l
              (boost::apply_visitor (*this, b.l));
            pnet::type::signature::signature_type r
              (boost::apply_visitor (*this, b.r));

            switch (b.token)
            {
            case ::expr::token::_substr:
              if (not (l == pnet::type::signature::signature_type (std::string ("std::string"))
                      and
                       r == pnet::type::signature::signature_type (std::string ("long"))
                      )
                 )
              {
                throw pnet::exception::type_error
                  ( ( boost::format ("%1% for types '%2%' and '%3%'")
                    % ::expr::token::show (b.token)
                    % pnet::type::signature::show (l)
                    % pnet::type::signature::show (r)
                    ).str()
                  );
              }
              break;
            case ::expr::token::_bitset_insert:
            case ::expr::token::_bitset_delete:
            case ::expr::token::_bitset_is_element:
              if (not (l == pnet::type::signature::signature_type (std::string ("bitset"))
                      and
                       r == pnet::type::signature::signature_type (std::string ("long"))
                      )
                 )
              {
                throw pnet::exception::type_error
                  ( ( boost::format ("%1% for types '%2%' and '%3%'")
                    % ::expr::token::show (b.token)
                    % pnet::type::signature::show (l)
                    % pnet::type::signature::show (r)
                    ).str()
                  );
              }
              break;
            default:
              if (not (l == r))
              {
                throw pnet::exception::type_error
                  ( ( boost::format ("%1% for types '%2%' and '%3%'")
                    % ::expr::token::show (b.token)
                    % pnet::type::signature::show (l)
                    % pnet::type::signature::show (r)
                    ).str()
                  );
              }
              break;
            }

            return std::string ("void");
          }
          pnet::type::signature::signature_type
          operator() (const ::expr::parse::node::ternary_t& t) const
          {
            return std::string ("void");
          }

        private:
          const pnet::type::signature::resolver_type& _resolve;
        };
      }

      pnet::type::signature::signature_type
      calculate ( const pnet::type::signature::resolver_type& resolve
                , const ::expr::parse::node::type& nd
                )
      {
        return boost::apply_visitor (visitor_calculate (resolve), nd);
      }
    }
  }
}
