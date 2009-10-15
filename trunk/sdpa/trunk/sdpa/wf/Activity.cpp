#include "Activity.hpp"

using namespace sdpa::wf;

Activity::Activity(const std::string &a_name, const Method &a_method, const parameter_list_t & params)
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

/*
  // build parameter list
  for (gwes::parameter_list_t::iterator it=parameters->begin(); it!=parameters->end(); ++it) {
    switch (it->scope) {
      case (TokenParameter::SCOPE_READ):
      case (TokenParameter::SCOPE_INPUT):
      case (TokenParameter::SCOPE_WRITE):
        continue;
      case (TokenParameter::SCOPE_OUTPUT):
        it->tokenP = new Token(new Data(string("<data><output xmlns=\"\">15</output></data>")));
        LOG_INFO(logger, "generated dummy output token: \n" << *it->tokenP);
        break;
    }
  }
*/
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
  params_.push_back(p);
}

void Activity::writeTo(std::ostream &os) const {
  os << name() << ":" << method();
 
  os << "(";
  for (parameter_list_t::const_iterator p(parameters().begin()); p != parameters().end(); p++) {
    os << *p;
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
