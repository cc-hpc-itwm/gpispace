#include "SedaMessage.hpp"

#include <sstream>

using namespace seda::comm;

std::string SedaMessage::str() const {
  if (strrep_buf_.empty())
  {
    std::ostringstream os;
    os << from() << " --> " << to() << ": '" << payload() << "'";
    strrep_buf_ = os.str();
  }
  return strrep_buf_;
}

