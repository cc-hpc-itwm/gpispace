#include "Activity.hpp"

#include <fhglog/fhglog.hpp>

// gwes
#include <gwes/TransitionOccurrence.h>
#include <gwes/Activity.h> // class Activity to provide wrapping

using namespace sdpa::wf;

Activity::Activity(const std::string &a_name, const Method &a_method, const parameters_t & params)
  : name_(a_name)
  , method_(a_method)
  , params_(params)
{
}

Activity::Activity(const gwes::Activity &gwes_activity)
  : name_(gwes_activity.getID())
  , method_("", "")
  , params_()
{
  DLOG(DEBUG, "wrapping an gwes::Activity");

  // FIXME: getters on gwes::Activity should be const
  gwes::Activity &act = const_cast<gwes::Activity&>(gwes_activity);
  if (act.getOperationCandidate())
  {
    method_ = Method(act.getOperationCandidate()->getOperationName());
    if (method_.module().empty() || method_.name().empty())
    {
      throw std::runtime_error("could not parse operation name into Method description: " + act.getOperationCandidate()->getOperationName());
    }
  }

  // build parameter list
  gwes::parameter_list_t *gwes_params = act.getTransitionOccurrence()->getTokens();
  for (gwes::parameter_list_t::iterator it(gwes_params->begin()); it != gwes_params->end(); ++it) {
    const std::string gwes_param_name(it->edgeP->getExpression());

    Token tok(*it->tokenP);
  
    switch (it->scope) {
      case (gwes::TokenParameter::SCOPE_READ):
      {
        Parameter p(gwes_param_name, Parameter::READ_EDGE, tok);
        DLOG(DEBUG, "read " << p);
        add_parameter(p);
        break;
      }
      case (gwes::TokenParameter::SCOPE_INPUT):
      {
        Parameter p(gwes_param_name, Parameter::INPUT_EDGE, tok);
        DLOG(DEBUG, "input " << p);
        add_parameter(p);
        break;
      }
      case (gwes::TokenParameter::SCOPE_WRITE):
      {
        Parameter p(gwes_param_name, Parameter::WRITE_EDGE, tok);
        DLOG(DEBUG, "write " << p);
        add_parameter(p);
        break;
      }
      case (gwes::TokenParameter::SCOPE_OUTPUT):
      {
        Parameter p(gwes_param_name, Parameter::OUTPUT_EDGE, tok);
        DLOG(DEBUG, "output " << p);
        add_parameter(p);
        break;
      }
      default:
      {
        DLOG(ERROR, "unknown parameter type: " << gwes_param_name << "(" << tok << ")" );
        break;
      }
    }
  }
}


Activity::Activity(const Activity &other)
  : name_(other.name())
  , method_(other.method())
  , params_(other.parameters())
  , properties_(other.properties())
{
}

Activity& Activity::operator=(const Activity &rhs) {
  if (this != &rhs)
  {
    name_ = rhs.name();
    method_ = rhs.method();
    params_ = rhs.parameters();
    properties_ = rhs.properties();
  }
  return *this;
}

void Activity::add_parameter(const Parameter &p) {
  params_.insert(std::make_pair(p.name(), p));
}

void Activity::writeTo(std::ostream &os) const {
  os << "{"
     << "act"
     << ","
     << name()
     << ","
     << method()
     << ","
     << "{" << "params"
     << ","
     << "[";
  for (parameters_t::const_iterator p(parameters().begin());;) {
    os << p->second;
    ++p;
    if (p == parameters().end())
    {
      break;
    }
    else
    {
      os << ",";
    }
  }
  os << "]"
     << "}"
     << "}";
}

std::ostream & operator<<(std::ostream & os, const sdpa::wf::Activity &a) {
  a.writeTo(os);
  return os;
}

std::ostream & operator<<(std::ostream & os, const sdpa::wf::Activity::Method &m) {
  m.writeTo(os);
  return os;
}
