// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/type/calculate.hpp>
#include <we/expr/token/type.hpp>

#include <we/type/signature/show.hpp>

#include <we/signature_of.hpp>
#include <we/exception.hpp>

#include <fhg/util/join.hpp>

#include <boost/format.hpp>

#include <stdexcept>

namespace pnet
{
  namespace expr
  {
    namespace type
    {
      using pnet::type::signature::signature_type;

      namespace
      {
        signature_type require
          ( std::string const& sig_l
          , std::string const& sig_r
          , ::expr::token::type const& token
          , signature_type const& l
          , signature_type const& r
          , std::string const& sig
          )
        {
          if (!(l == signature_type (sig_l) && r == signature_type (sig_r)))
          {
            throw pnet::exception::type_error
              ( ( boost::format ("'%1%' for types '%2%' and '%3%'")
                % ::expr::token::show (token)
                % pnet::type::signature::show (l)
                % pnet::type::signature::show (r)
                ).str()
              );
          }

          return sig;
        }

        class visitor_calculate : public boost::static_visitor<signature_type>
        {
        public:
          visitor_calculate (resolver_map_type& resolver_map)
            : _resolver_map (resolver_map)
          {}

          signature_type
            operator() (const pnet::type::value::value_type& v) const
          {
            return pnet::signature_of (v);
          }
          signature_type
            operator() (const std::list<std::string>& path) const
          {
            return resolve (path);
          }
          signature_type
            operator() (const ::expr::parse::node::unary_t& u) const
          {
            signature_type s0 (boost::apply_visitor (*this, u.child));

            return std::string ("void");
          }
          signature_type
            operator() (const ::expr::parse::node::binary_t& b) const
          {
            signature_type const l (boost::apply_visitor (*this, b.l));
            signature_type const r (boost::apply_visitor (*this, b.r));

            switch (b.token)
            {
            case ::expr::token::_or:
            case ::expr::token::_and:
              return require ("bool", "bool", b.token, l, r, "bool");

            case ::expr::token::_substr:
              return require ("string", "long", b.token, l, r, "string");

            case ::expr::token::_bitset_insert:
            case ::expr::token::_bitset_delete:
              return require ("bitset", "long", b.token, l, r, "bitset");

            case ::expr::token::_bitset_is_element:
              return require ("bitset", "long", b.token, l, r, "bool");

            default:
              break;
            }

            throw std::runtime_error
              ( ( boost::format ("Strange: Unknown token '%1%'"
                                " during calculation of expression type"
                                )
                % ::expr::token::show (b.token)
                ).str()
              );
          }
          signature_type
            operator() (const ::expr::parse::node::ternary_t& t) const
          {
            return std::string ("void");
          }

        private:
          resolver_map_type& _resolver_map;

          signature_type resolve (std::list<std::string> const& path) const
          {
            resolver_map_type::const_iterator const
              pos (_resolver_map.find (path));

            if (pos == _resolver_map.end())
            {
              throw std::runtime_error
                ( ( boost::format ("Could not resolve '%1%'")
                  % fhg::util::join (path, ".")
                  ).str()
                );
            }

            return pos->second;
          }
        };
      }

      signature_type
        calculate ( resolver_map_type& resolver_map
                  , const ::expr::parse::node::type& nd
                  )
      {
        return boost::apply_visitor (visitor_calculate (resolver_map), nd);
      }
    }
  }
}
