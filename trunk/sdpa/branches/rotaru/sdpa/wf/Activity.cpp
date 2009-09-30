#include "Activity.hpp"

using namespace sdpa::wf;

Activity::Activity(const std::string &name)
  : name_(name) {
}

Activity::Activity(const std::string &name, const Method &method)
  : name_(name), method_(method) {
}

Activity::Activity(const std::string &name, const Method &method, const parameter_list & input)
  : name_(name), method_(method), input_(input) {
}

Activity::Activity(const std::string &name, const Method &method, const parameter_list & input, const parameter_list & output)
  : name_(name), method_(method), input_(input), output_(output) {
}

Activity::Activity(const Activity &other)
  : name_(other.name()), method_(other.method()), input_(other.input()), output_(other.output()) {
}

const Activity & Activity::operator=(const Activity &rhs) {
  name_ = rhs.name();
  method_ = rhs.method();
  input_ = rhs.input();
  output_ = rhs.output();
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
  for (parameter_list::const_iterator p(input().begin()); p != input().end(); p++) {
    os << *p;
  }
  os << ")->[";

  for (parameter_list::const_iterator p(output().begin()); p != output().end(); p++) {
    os << *p;
  }
  os << "]";
}

std::ostream & operator<<(std::ostream & os, const sdpa::wf::Activity &a) {
  a.writeTo(os);
  return os;
}

