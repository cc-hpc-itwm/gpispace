#include "Token.hpp"

std::ostream & operator<<(std::ostream &os, const sdpa::Token &t) {
  t.writeTo(os);
  return os;
}
