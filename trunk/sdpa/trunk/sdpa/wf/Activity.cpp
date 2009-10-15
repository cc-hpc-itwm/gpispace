#include "Activity.hpp"

using namespace sdpa::wf;

Activity::Activity(const std::string &a_name, const Method &a_method, const parameter_list_t & params)
  : name_(a_name)
  , method_(a_method)
  , params_(params)
{
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


