#include "Parameter.hpp"

sdpa::Parameter::Parameter(const sdpa::Token &token, const std::string &name, EdgeType edge_type)
  : token_(token), name_(name), edge_type_(edge_type) {
  }

sdpa::Parameter::Parameter(const sdpa::Parameter &other)
  : token_(other.token()), name_(other.name()), edge_type_(other.edge_type()) {
  }

const sdpa::Parameter& sdpa::Parameter::operator=(const sdpa::Parameter &rhs) {
  token_ = rhs.token();
  name_ = rhs.name();
  edge_type_ = rhs.edge_type();
}

void sdpa::Parameter::writeTo(std::ostream &os) const {
  switch (edge_type()) {
    case INPUT_EDGE:
      os << "i";
      break;
    case READ_EDGE:
      os << "r";
      break;
    case OUTPUT_EDGE:
      os << "o";
      break;
    case WRITE_EDGE:
      os << "w";
      break;
    case EXCHANGE_EDGE:
      os << "x";
      break;
    case UPDATE_EDGE:
      os << "u";
      break;
  }
  os << ":";
  os << name() << "=" << token();
}

std::ostream & operator<<(std::ostream & os, const sdpa::Parameter &p) {
  p.writeTo(os);
  return os;
}
