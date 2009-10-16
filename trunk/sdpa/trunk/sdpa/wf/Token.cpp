#include "Token.hpp"
#include <gwdl/Token.h>
#include <fhglog/fhglog.hpp>

using namespace sdpa::wf;

std::ostream & operator<<(std::ostream &os, const Token &t) {
  t.writeTo(os);
  return os;
}

Token::Token(const gwdl::Token &gwdl_token) throw (std::exception)
  : data_("")
#ifdef ENABLE_TYPE_CHECKING
  , type_("")
#endif
  , properties_()
{
  // fix const problems
  DLOG(DEBUG, "FIXME: const_cast<gwdl::Token> should not be needed!");
  gwdl::Token &gtok(const_cast<gwdl::Token&>(gwdl_token));
  if (gtok.isData())
  {
    data(*gtok.getData()->getText());
    // fix the type information
    properties().put("datatype", typeid(std::string).name());
#ifdef ENABLE_TYPE_CHECKING
    type_ = typeid(std::string).name();
#endif
  }
  else
  {
    // control token
    properties().put("datatype", typeid(void).name());
    properties().put("control", gtok.getControl());
#ifdef ENABLE_TYPE_CHECKING
    type_ = typeid(void).name();
#endif
  }
}
