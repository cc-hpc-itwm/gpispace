#include "Activity.hpp"

using namespace sdpa;

Activity::Activity(const std::string &name, const std::string &module_name, const std::string &method_name)
  : name_(name), module_name_(module_name), method_name_(method_name) {
}

Activity::Activity(const std::string &name, const std::string &module_name, const std::string &method_name, const parameter_list & input)
  : name_(name), module_name_(module_name), method_name_(method_name), input_(input) {
}

Activity::Activity(const std::string &name, const std::string &module_name, const std::string &method_name, const parameter_list & input, const parameter_list & output)
  : name_(name), module_name_(module_name), method_name_(method_name), input_(input), output_(output) {
}

Activity::Activity(const Activity &other)
  : name_(other.name()), module_name_(other.module_name()), method_name_(other.method_name()), input_(other.input()), output_(other.output()) {
}

const Activity & Activity::operator=(const Activity &rhs) {
  name_ = rhs.name();
  module_name_ = rhs.module_name();
  method_name_ = rhs.method_name();
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
  os << name() << ":" << module_name() << "::" << method_name();
 
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

std::ostream & operator<<(std::ostream & os, const sdpa::Activity &a) {
  a.writeTo(os);
  return os;
}

