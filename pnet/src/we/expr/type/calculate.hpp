// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_EXPR_TYPE_CALCULATE
#define PNET_SRC_WE_EXPR_TYPE_CALCULATE

#include <we/type/signature.hpp>
#include <we/type/signature/resolve.hpp>

#include <we/expr/parse/node.hpp>

namespace pnet
{
  namespace expr
  {
    namespace type
    {
      pnet::type::signature::signature_type calculate
        ( const pnet::type::signature::resolver_type&
        , const ::expr::parse::node::type&
        );
    }
  }
}

#endif
