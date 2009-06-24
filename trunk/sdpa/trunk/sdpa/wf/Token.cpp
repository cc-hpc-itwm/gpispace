#include "Token.hpp"

std::ostream & operator<<(std::ostream &os, const sdpa::wf::Token &t) {
  t.writeTo(os);
  return os;
}
