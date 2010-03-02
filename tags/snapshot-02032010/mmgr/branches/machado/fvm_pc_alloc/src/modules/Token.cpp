#include "Token.hpp"

using namespace sdpa::wf;

std::ostream & operator<<(std::ostream &os, const Token &t) {
  t.writeTo(os);
  return os;
}
