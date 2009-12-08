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

#include <sdpa/types.hpp>

#include <sdpa/wf/Token.hpp>
#include <sdpa/wf/Activity.hpp>

// gwes
#include <gwdl/Workflow.h>
#include <gwdl/Token.h>
#include <gwes/TransitionOccurrence.h>
#include <gwes/Activity.h>

namespace sdpa { namespace wf { namespace glue {
  inline
  Token wrap(const gwdl::Token & gwdl_token) throw (std::exception)
  {
    Token wrapped;

    // fix const problems
    DLOG(DEBUG, "FIXME: const_cast<gwdl::Token> should not be needed!");
    gwdl::Token &gtok(const_cast<gwdl::Token&>(gwdl_token));
    if (gtok.isData())
    {
      typedef std::map<std::string, std::string> gtok_properties_t;

      std::string gwdl_token_data(gtok.getData()->getContent());

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

      wrapped.data(gtok.getData()->getContent());
      for (gtok_properties_t::const_iterator prop(gtok.getProperties()->begin()); prop != gtok.getProperties()->end(); ++prop)
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

  inline
  gwdl::Token *unwrap(const wf::Token &wf_token) throw (std::exception)
  {
    DLOG(DEBUG, "unwrapping wf::Token to gwdl::Token...");
    if (wf_token.properties().has_key("control"))
    {
	  bool control_value = wf_token.properties().get<bool>("control");
      gwdl::Token *tok = new gwdl::Token( (control_value ? gwdl::Token::CONTROL_TRUE : gwdl::Token::CONTROL_FALSE) );
      for (wf::Token::properties_t::const_iterator prop(wf_token.properties().begin()); prop != wf_token.properties().end(); ++prop)
      {
        (*tok->getProperties())[prop->first] = prop->second;
      }
      return tok;
    }
    else
    {
      gwdl::Data::ptr_t data(new gwdl::Data(wf_token.data()));
      gwdl::Token *tok = new gwdl::Token(data);

      // update all known properties, this also includes the data type
      for (wf::Token::properties_t::const_iterator prop(wf_token.properties().begin()); prop != wf_token.properties().end(); ++prop)
      {
        (*tok->getProperties())[prop->first] = prop->second;
      }
      return tok;
    }
  }

  inline
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

	// TODO: wrap the properties!!!

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

  inline
  void unwrap(const wf::Activity &wf_activity, gwes::Activity &gwes_activity)
  {
    if (wf_activity.name() != gwes_activity.getID())
    {
      throw std::runtime_error("could not unwrap " + wf_activity.name() + ": does not match gwes::Activity::id");
    }

    DLOG(DEBUG, "unwrapping wf::Activity: " << wf_activity.name() << " to " << gwes_activity.getID());

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
            DLOG(DEBUG, "removing old Token on output: " << param->tokenP);
//            delete param->tokenP; param->tokenP = NULL;
          }
          param->tokenP = gwdl::Token::ptr_t(unwrap(wf_activity.parameters().at(gwes_param_name).token()));
          break;
        }
      }
    }
  }

  inline
  sdpa::job_result_t wrap(const gwdl::workflow_result_t &workflow_results)
  {
    DLOG(DEBUG, "wrapping workflow results...");
    job_result_t result;

    for (gwdl::workflow_result_t::const_iterator place_to_tokens(workflow_results.begin()); place_to_tokens != workflow_results.end(); ++place_to_tokens)
    {
      const std::string place_name(place_to_tokens->first);
      const gwdl::token_list_t &gwdl_tokens(place_to_tokens->second);

      DLOG(DEBUG, "wrapping " << gwdl_tokens.size() << " token(s) on place " << place_name);

      sdpa::token_list_t tokens;
      for (gwdl::token_list_t::const_iterator a_token(gwdl_tokens.begin()); a_token != gwdl_tokens.end(); ++a_token)
      {
        gwdl::Token *gwdl_token = dynamic_cast<gwdl::Token*>(a_token->get());
        if (gwdl_token)
        {
          tokens.push_back(wrap(*gwdl_token));
        }
        else
        {
          LOG(ERROR, "the gwdl::token_list did not contain gwdl::Token, ignoring this one.");
        }
      }

      result.insert(std::make_pair(place_name, tokens));
    }
    return result;
  }

  inline
  void put_results_to_activity(const sdpa::job_result_t &result, gwes::Activity &gwes_activity)
  {
    DLOG(DEBUG, "putting results back to activity: " << gwes_activity.getID());

    gwes::parameter_list_t *gwes_params = gwes_activity.getTransitionOccurrence()->getTokens();
    for (gwes::parameter_list_t::iterator param(gwes_params->begin()); param != gwes_params->end(); ++param)
    {
      const std::string gwes_param_name(param->edgeP->getExpression());
      switch (param->scope)
      {
        case gwes::TokenParameter::SCOPE_READ:
        {
          // nothing to do
          DLOG(DEBUG, "putting read parameter: nothing to do");
          break;
        }
        case (gwes::TokenParameter::SCOPE_INPUT):
        {
          // nothing to do
          DLOG(DEBUG, "putting input parameter: nothing to do");
          break;
        }
        case (gwes::TokenParameter::SCOPE_WRITE):
        {
          DLOG(DEBUG, "putting write parameter: replacing token");
          if (param->tokenP)
          {
            DLOG(DEBUG, "removing old Token on output: " << param->tokenP);
//            delete param->tokenP; param->tokenP = NULL;
          }
          // looking up tokens in result
          // TODO: check for existence!
          const sdpa::token_list_t &result_tokens = result.at(param->edgeP->getPlaceID());
          if (result_tokens.size() == 1)
          {
            param->tokenP = gwdl::Token::ptr_t(unwrap(result_tokens.front()));
          }
          else if (result_tokens.size() > 1)
          {
            LOG(ERROR, "more than one result token is currently not supported!");
            throw std::runtime_error("more than one result token is currently not supported");
          }
          else
          {
            LOG(ERROR, "expected a token on write edge: " << gwes_param_name);
            throw std::runtime_error("expected a token on " + gwes_param_name);
          }
          break;
        }
        case (gwes::TokenParameter::SCOPE_OUTPUT):
        {
          DLOG(DEBUG, "putting output parameter: creating new token");
          if (param->tokenP)
          {
            DLOG(DEBUG, "removing old Token on output: " << param->tokenP);
//            delete param->tokenP; param->tokenP = NULL;
          }
          // looking up tokens in result
          // TODO: check for existence!
          const sdpa::token_list_t &result_tokens = result.at(param->edgeP->getPlaceID());
          if (result_tokens.size() == 1)
          {
            param->tokenP = gwdl::Token::ptr_t(unwrap(result_tokens.front()));
          }
          else if (result_tokens.size() > 1)
          {
            LOG(ERROR, "more than one result token is currently not supported!");
            throw std::runtime_error("more than one result token is currently not supported");
          }
          else
          {
            LOG(ERROR, "expected a token on output edge: " << gwes_param_name);
            throw std::runtime_error("expected a token on " + gwes_param_name);
          }
          break;
        }
      }
    }
  }
}}}

#endif
