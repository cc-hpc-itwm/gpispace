#include "Parameter.hpp"

using namespace sdpa;

Parameter::Parameter(const Token &token, const std::string &name, EdgeType edge_type)
  : token_(token), name_(name), edge_type_(edge_type) {
}

Parameter::Parameter(const Parameter &other)
  : token_(other.const_token()), name_(other.name()), edge_type_(other.edge_type()) {
}

const Parameter& Parameter::operator=(const Parameter &rhs) {
  token_ = rhs.const_token();
  name_ = rhs.name();
  edge_type_ = rhs.edge_type();
}
