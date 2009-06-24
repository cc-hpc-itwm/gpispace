#include "Token.hpp"

std::ostream & operator<<(std::ostream &os, const sdpa::daemon::Token &t) {
  t.writeTo(os);
  return os;
}
