// mirko.rahn@itwm.fraunhofer.de

#ifndef PNET_SRC_WE_EXPR_TYPE_CALCULATE
#define PNET_SRC_WE_EXPR_TYPE_CALCULATE

#include <we/type/signature.hpp>

#include <we/expr/parse/node.hpp>

#include <boost/unordered_map.hpp>

namespace pnet
{
  namespace expr
  {
    namespace type
    {
      typedef boost::unordered_map< std::list<std::string>
                                  , pnet::type::signature::signature_type
                                  > resolver_map_type;

      pnet::type::signature::signature_type
        calculate (resolver_map_type&, const ::expr::parse::node::type&);
    }
  }
}

#endif
