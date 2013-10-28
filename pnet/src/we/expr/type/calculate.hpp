// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_EXPR_TYPE_CALCULATE
#define PNET_SRC_WE_EXPR_TYPE_CALCULATE

#include <we/type/signature.hpp>

#include <we/expr/parse/node.hpp>

#include <boost/function.hpp>

namespace pnet
{
  namespace expr
  {
    namespace type
    {
      typedef boost::function
        < pnet::type::signature::signature_type (const std::list<std::string>&)
        > resolver_type;

      pnet::type::signature::signature_type
        calculate (const resolver_type&, const ::expr::parse::node::type&);
    }
  }
}

#endif
