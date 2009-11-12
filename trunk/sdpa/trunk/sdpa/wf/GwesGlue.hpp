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
#include <gwdl/Workflow.h>
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
      typedef std::map<std::string, std::string> gtok_properties_t;

      std::string gwdl_token_data(*gtok.getData()->getText());

      {
        // remove sdpa-tag...
        const std::string sTag("<sdpa>");
        const std::string eTag("</sdpa>");
        if (gwdl_token_data.substr(0, sTag.size()) == sTag)
        {
          DLOG(DEBUG, "removing enclosing sdpa-tags...");
          try
          {
            gwdl_token_data = gwdl_token_data.substr(sTag.size(), gwdl_token_data.size() - (sTag.size()+eTag.size()) );
          }
          catch (const std::exception &ex)
          {
            LOG(ERROR, "malformed token-data: could not remove enclosing sdpa-tags: " << ex.what());
            throw;
          }
        }
        else
        {
          DLOG(DEBUG, "no sdpa-tag found, taking data as it is: " << gwdl_token_data);
        }
      }

      wrapped.data(*gtok.getData()->getText());
      for (gtok_properties_t::const_iterator prop(gtok.getProperties().begin()); prop != gtok.getProperties().end(); ++prop)
      {
        wrapped.properties().put(prop->first, prop->second);
      }
      if (! wrapped.properties().has_key("datatype"))
      {
        DLOG(DEBUG, "fixing datatype information of Token, assuming string type");
        // fix the type information
        wrapped.properties().put("datatype", typeid(std::string).name());
      }
    }
    else
    {
      // control token
      wrapped.properties().put("datatype", typeid(void).name());
      wrapped.properties().put("control", gtok.getControl());
    }
    return wrapped;
  }

  gwdl::Token *unwrap(const wf::Token &wf_token) throw (std::exception)
  {
    DLOG(DEBUG, "unwrapping wf::Token to gwdl::Token...");
    if (wf_token.properties().has_key("control"))
    {
      gwdl::Token *tok = new gwdl::Token(wf_token.properties().get<bool>("control"));
      for (wf::Token::properties_t::const_iterator prop(wf_token.properties().begin()); prop != wf_token.properties().end(); ++prop)
      {
        tok->getProperties()[prop->first] = prop->second;
      }
      return tok;
    }
    else
    {
      gwdl::Data *data(new gwdl::Data("<data><sdpa>"+wf_token.data()+"</sdpa></data>"));
      gwdl::Token *tok = new gwdl::Token(data);

      // update all known properties, this also includes the data type
      for (wf::Token::properties_t::const_iterator prop(wf_token.properties().begin()); prop != wf_token.properties().end(); ++prop)
      {
        tok->getProperties()[prop->first] = prop->second;
      }
      return tok;
    }
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
      wrapped.method() = Method(act.getOperationCandidate()->getOperationName());
      if (wrapped.method().module().empty() || wrapped.method().name().empty())
      {
        throw std::runtime_error("could not parse operation name into Method description: " + act.getOperationCandidate()->getOperationName());
      }
    }

    // build parameter list
    gwes::parameter_list_t *gwes_params = act.getTransitionOccurrence()->getTokens();
    for (gwes::parameter_list_t::iterator it(gwes_params->begin()); it != gwes_params->end(); ++it) {
      const std::string gwes_param_name(it->edgeP->getExpression());

      Token tok;
      if (it->tokenP)
      {
        DLOG(DEBUG, "wrapping gwdl::Token");
        tok = wrap(*it->tokenP);
      }

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

  void unwrap(const wf::Activity &wf_activity, gwes::Activity &gwes_activity)
  {
    if (wf_activity.name() != gwes_activity.getID())
    {
      throw std::runtime_error("could not unwrap " + wf_activity.name() + ": does not match gwes::Activity::id");
    }

    DLOG(INFO, "unwrapping wf::Activity: " << wf_activity.name() << " to " << gwes_activity.getID());

    gwes::parameter_list_t *gwes_params = gwes_activity.getTransitionOccurrence()->getTokens();
    for (gwes::parameter_list_t::iterator param(gwes_params->begin()); param != gwes_params->end(); ++param)
    {
      const std::string gwes_param_name(param->edgeP->getExpression());
      switch (param->scope)
      {
        case gwes::TokenParameter::SCOPE_READ:
        {
          // nothing to do
          DLOG(DEBUG, "unwrapping read parameter: nothing to do");
          break;
        }
        case (gwes::TokenParameter::SCOPE_INPUT):
        {
          // nothing to do
          DLOG(DEBUG, "unwrapping input parameter: nothing to do");
          break;
        }
        case (gwes::TokenParameter::SCOPE_WRITE):
        {
          DLOG(DEBUG, "unwrapping write parameter: nothing to do");
          break;
        }
        case (gwes::TokenParameter::SCOPE_OUTPUT):
        {
          DLOG(DEBUG, "unwrapping output parameter: creating new token");
          if (param->tokenP)
          {
            DLOG(DEBUG, "removing old Token on output: " << *param->tokenP);
            delete param->tokenP; param->tokenP = NULL;
          }
          param->tokenP = unwrap(wf_activity.parameters().at(gwes_param_name).token());
          break;
        }
      }
    }
  }

  // extract tokens from workflow
  // returns a map of place-id to list-of-tokens
  typedef std::list<sdpa::wf::Token> token_list_t;
  typedef std::map<std::string, token_list_t> workflow_result_t;
  typedef std::vector<gwdl::Token*> gwdl_token_list_t;

  workflow_result_t get_workflow_results(const gwdl::Workflow &const_workflow)
  {
    DLOG(INFO, "retrieving results from workflow: " << const_workflow.getID());

    LOG(WARN, "FIXME: removing const, this should not be necessary!");
    gwdl::Workflow &workflow = const_cast<gwdl::Workflow&>(const_workflow);
    
    workflow_result_t result;

    // iterate over all places
    typedef std::vector<std::string> place_names_t;
    place_names_t place_names = workflow.getPlaceIDs();
    for (place_names_t::const_iterator p_name(place_names.begin()); p_name != place_names.end(); ++p_name)
    {
      try
      {
        gwdl::Place *place = workflow.getPlace(*p_name);
        DLOG(DEBUG, "getting tokens from place: " << *p_name);
        token_list_t tokens;
        {
          gwdl_token_list_t gwdl_tokens = place->getTokens();
          for (gwdl_token_list_t::const_iterator gwdl_token(gwdl_tokens.begin()); gwdl_token != gwdl_tokens.end(); ++gwdl_token)
          {
            tokens.push_back(wrap(**gwdl_token));
          }
        }
        DLOG(DEBUG, "found " << tokens.size() << " tokens on place " << *p_name);

        result[*p_name] = tokens;
      }
      catch (const gwdl::NoSuchWorkflowElement &)
      {
        LOG(WARN, "Inconsistencey detected: the workflow does not contain place: " << *p_name);
      }
    }

    return result;
  }
}}}

#endif
