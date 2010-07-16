#include "Parameter.hpp"

using namespace sdpa::wf;

Parameter::Parameter()
  : name_("unknown")
  , edge_type_(OUTPUT_EDGE)
  , token_()
{

}
Parameter::Parameter(const std::string &a_name, EdgeType a_edge_type, const Token &a_token)
  : name_(a_name)
  , edge_type_(a_edge_type)
  , token_(a_token)
{
}

Parameter::Parameter(const Parameter &other)
  : name_(other.name())
  , edge_type_(other.edge_type())
  , token_(other.token())
{
}

Parameter& Parameter::operator=(const Parameter &rhs)
{
  if (this != &rhs)
  {
    name_ = rhs.name();
    edge_type_ = rhs.edge_type();
    token_ = rhs.token();
  }
  return *this;
}

void Parameter::writeTo(std::ostream &os) const {
  os << "{"
     << "param"
     << ","
     << name()
     << ",";
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
  os << ","
     << token()
     << "}";
}

std::ostream & operator<<(std::ostream & os, const sdpa::wf::Parameter &p) {
  p.writeTo(os);
  return os;
}
