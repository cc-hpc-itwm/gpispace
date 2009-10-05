#include "uuid.hpp"
#include <sstream>
#include <cstring> // memcpy
#include <iomanip>

using namespace sdpa;

uuid::uuid(const uuid &other)
  : dirty_(true)
  , str_tmp_("")
{
  set(other.data());
}

uuid::uuid(const uuid_t &some_data)
  : dirty_(true)
  , str_tmp_("")
{
  set(some_data);
}

uuid::uuid()
  : dirty_(true)
  , str_tmp_("")
{
}

uuid::~uuid() {}

uuid &uuid::operator=(const uuid &other) {
  if (this != &other) {
    set(other.data());
  }
  return *this;
}

void uuid::set(const uuid_t &a_data) {
  dirty_ = true;
  std::memcpy(uuid_, a_data, sizeof(uuid_t));
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
