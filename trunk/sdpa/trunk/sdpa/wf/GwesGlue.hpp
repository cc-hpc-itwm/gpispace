/*
 * =====================================================================================
 *
 *       Filename:  GwesGlue.hpp
 *
 *    Description:  glue code for interacting with gwes
 *
 *        Version:  1.0
 *        Created:  10/17/2009 02:34:45 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_WF_GWES_GLUE_HPP
#define SDPA_WF_GWES_GLUE_HPP 1

#include <fhglog/fhglog.hpp>

#include <sdpa/wf/Token.hpp>
#include <gwdl/Token.h>

namespace sdpa { namespace wf { namespace glue {
  Token wrap(const gwdl::Token & gwdl_token)
  {
    Token wrapped;

    // fix const problems
    DLOG(DEBUG, "FIXME: const_cast<gwdl::Token> should not be needed!");
    gwdl::Token &gtok(const_cast<gwdl::Token&>(gwdl_token));
    if (gtok.isData())
    {
      wrapped.data(*gtok.getData()->getText());
      // fix the type information
      wrapped.properties().put("datatype", typeid(std::string).name());
    }
    else
    {
      // control token
      wrapped.properties().put("datatype", typeid(void).name());
      wrapped.properties().put("control", gtok.getControl());
    }
    return wrapped;
  }
}}}

#endif
