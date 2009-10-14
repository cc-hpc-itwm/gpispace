#include "Activity.hpp"

using namespace sdpa::wf;

Activity::Activity(const std::string &a_name, const Method &a_method, const parameter_list_t & a_input, const parameter_list_t & a_output)
  : name_(a_name)
  , method_(a_method)
  , input_(a_input)
  , output_(a_output)
{
}

Activity::Activity(const Activity &other)
  : name_(other.name())
  , method_(other.method())
  , input_(other.input())
  , output_(other.output())
{
}

Activity& Activity::operator=(const Activity &rhs) {
  if (this != &rhs)
  {
    name_ = rhs.name();
    method_ = rhs.method();
    input_ = rhs.input();
    output_ = rhs.output();
  }
  return *this;
}

void Activity::add_input(const Parameter &p) {
  input_.push_back(p);
}

void Activity::add_output(const Parameter &p) {
  output_.push_back(p);
}

void Activity::writeTo(std::ostream &os) const {
  os << name() << ":" << method().module() << "::" << method().name();
 
  os << "(";
  for (parameter_list_t::const_iterator p(input().begin()); p != input().end(); p++) {
    os << *p;
  }
  os << ")->[";

  for (parameter_list_t::const_iterator p(output().begin()); p != output().end(); p++) {
    os << *p;
  }
  os << "]";
}

std::ostream & operator<<(std::ostream & os, const sdpa::wf::Activity &a) {
  a.writeTo(os);
  return os;
}

