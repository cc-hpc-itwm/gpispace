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

#include <stdexcept>

#include <sdpa/wf/Token.hpp>
#include <sdpa/wf/Activity.hpp>

// gwes
#include <gwdl/Token.h>
#include <gwes/TransitionOccurrence.h>
#include <gwes/Activity.h>

namespace sdpa { namespace wf { namespace glue {
  Token wrap(const gwdl::Token & gwdl_token) throw (std::exception)
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

  Activity wrap(const gwes::Activity & gwes_activity) throw (std::exception)
  {
    DLOG(DEBUG, "wrapping an gwes::Activity");
    Activity wrapped;
    wrapped.name() = gwes_activity.getID();

    // FIXME: getters on gwes::Activity should be const
    gwes::Activity &act = const_cast<gwes::Activity&>(gwes_activity);
    if (act.getOperationCandidate())
    {
      wrapped.method() = Activity::Method(act.getOperationCandidate()->getOperationName());
      if (wrapped.method().module().empty() || wrapped.method().name().empty())
      {
        throw std::runtime_error("could not parse operation name into Method description: " + act.getOperationCandidate()->getOperationName());
      }
    }

    // build parameter list
    gwes::parameter_list_t *gwes_params = act.getTransitionOccurrence()->getTokens();
    for (gwes::parameter_list_t::iterator it(gwes_params->begin()); it != gwes_params->end(); ++it) {
      const std::string gwes_param_name(it->edgeP->getExpression());

      Token tok(wrap(*it->tokenP));

      switch (it->scope) {
        case (gwes::TokenParameter::SCOPE_READ):
          {
            Parameter p(gwes_param_name, Parameter::READ_EDGE, tok);
            DLOG(DEBUG, "read " << p);
            wrapped.add_parameter(p);
            break;
          }
        case (gwes::TokenParameter::SCOPE_INPUT):
          {
            Parameter p(gwes_param_name, Parameter::INPUT_EDGE, tok);
            DLOG(DEBUG, "input " << p);
            wrapped.add_parameter(p);
            break;
          }
        case (gwes::TokenParameter::SCOPE_WRITE):
          {
            Parameter p(gwes_param_name, Parameter::WRITE_EDGE, tok);
            DLOG(DEBUG, "write " << p);
            wrapped.add_parameter(p);
            break;
          }
        case (gwes::TokenParameter::SCOPE_OUTPUT):
          {
            Parameter p(gwes_param_name, Parameter::OUTPUT_EDGE, tok);
            DLOG(DEBUG, "output " << p);
            wrapped.add_parameter(p);
            break;
          }
        default:
          {
            DLOG(ERROR, "unknown parameter type: " << gwes_param_name << "(" << tok << ")" );
            break;
          }
      }
    }
    return wrapped;
  }
}}}

#endif
