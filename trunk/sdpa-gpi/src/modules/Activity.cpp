#include "Activity.hpp"

//#include <fhglog/fhglog.hpp>

using namespace sdpa::wf;

Activity::Activity(const std::string &a_name, const Method &a_method, const parameters_t & params)
  : name_(a_name)
  , method_(a_method)
  , params_(params)
{
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
