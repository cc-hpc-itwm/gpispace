#include "Activity.hpp"

#include <fhglog/fhglog.hpp>

// gwes
#include <gwes/TransitionOccurrence.h>
#include <gwes/Activity.h> // class Activity to provide wrapping

using namespace sdpa::wf;

Activity::Activity(const std::string &a_name, const Method &a_method, const parameters_t & params)
  : Properties()
  , name_(a_name)
  , method_(a_method)
  , params_(params)
{
}

Activity::Activity(const gwes::Activity &gwes_activity)
  : Properties()
  , name_(gwes_activity.getID())
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
    const std::string gwes_param_data( it->tokenP->isData() ? *(it->tokenP->getData()->getText()) : "control");

    switch (it->scope) {
      case (gwes::TokenParameter::SCOPE_READ):
        DLOG(DEBUG, "read " << gwes_param_name << "(" << gwes_param_data << ")" );
        add_parameter(Parameter(gwes_param_name, Parameter::READ_EDGE, Token(gwes_param_data)));
        break;
      case (gwes::TokenParameter::SCOPE_INPUT):
        DLOG(DEBUG, "input " << gwes_param_name << "(" << gwes_param_data << ")" );
        add_parameter(Parameter(gwes_param_name, Parameter::INPUT_EDGE, Token(gwes_param_data)));
        break;
      case (gwes::TokenParameter::SCOPE_WRITE):
        DLOG(DEBUG, "write " << gwes_param_name << "(" << gwes_param_data << ")" );
        add_parameter(Parameter(gwes_param_name, Parameter::WRITE_EDGE, Token(gwes_param_data)));
        break;
      case (gwes::TokenParameter::SCOPE_OUTPUT):
        DLOG(DEBUG, "output " << gwes_param_name << "(" << gwes_param_data << ")" );
        add_parameter(Parameter(gwes_param_name, Parameter::OUTPUT_EDGE, Token(gwes_param_data)));
        break;
      default:
        DLOG(DEBUG, "unknown " << gwes_param_name << "(" << gwes_param_data << ")" );
        break;
    }
  }
}


Activity::Activity(const Activity &other)
  : name_(other.name())
  , method_(other.method())
  , params_(other.parameters())
{
}

Activity& Activity::operator=(const Activity &rhs) {
  if (this != &rhs)
  {
    name_ = rhs.name();
    method_ = rhs.method();
    params_ = rhs.parameters();
  }
  return *this;
}

void Activity::add_parameter(const Parameter &p) {
  params_.insert(std::make_pair(p.name(), p));
}

void Activity::writeTo(std::ostream &os) const {
  os << name() << ":" << method();
 
  os << "(";
  for (parameters_t::const_iterator p(parameters().begin()); p != parameters().end(); p++) {
    os << p->second;
    if (p != parameters().end())
    {
      os << ", ";
    }
  }
  os << ")";
}

std::ostream & operator<<(std::ostream & os, const sdpa::wf::Activity &a) {
  a.writeTo(os);
  return os;
}

std::ostream & operator<<(std::ostream & os, const sdpa::wf::Activity::Method &m) {
  m.writeTo(os);
  return os;
}
