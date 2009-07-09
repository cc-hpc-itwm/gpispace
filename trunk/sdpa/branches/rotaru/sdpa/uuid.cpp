#include "uuid.hpp"
#include <sstream>
#include <cstring> // memcpy
#include <iomanip>

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
    sstr << std::hex << std::right;
    for (std::size_t i(0); i < sizeof(uuid_t); ++i) {
       int c = (int(uuid_[i]) & 0xff);
       sstr << std::setw(2) << std::setfill('0') << c;
    }
    str_tmp_ = sstr.str();
    dirty_ = false;
  }
  return str_tmp_;
}
