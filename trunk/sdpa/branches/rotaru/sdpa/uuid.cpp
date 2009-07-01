#include "uuid.hpp"
#include <sstream>
#include <cstring> // memcpy

using namespace sdpa;

uuid::uuid(const uuid &other) {
  set(other.data());
}

uuid::uuid(const uuid_t &data) {
  set(data);
}

uuid::uuid() {}

uuid::~uuid() {}

const uuid &uuid::operator=(const uuid &other) {
  if (this != &other) {
    set(other.data());
  }
  return *this;
}

void uuid::set(const uuid_t &data) {
  dirty_ = true;
  std::memcpy(uuid_, data, sizeof(uuid_t));
}

const std::string &uuid::str() const {
  if (dirty_) {
    std::stringstream sstr;
    sstr << std::hex;
    for (std::size_t i(0); i < sizeof(uuid_t); ++i) {
      sstr << (uuid_[i] & 0xff);
    }
    str_tmp_ = sstr.str();
  }
  return str_tmp_;
}
