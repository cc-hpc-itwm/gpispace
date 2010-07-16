#include "Properties.hpp"

void sdpa::util::Properties::put(const std::string &key, const std::string &val) {
    del(key);
    properties_.insert(std::make_pair(key, val));
}

std::size_t sdpa::util::Properties::del(const std::string &key) {
    return properties_.erase(key);
}

bool sdpa::util::Properties::has_key(const std::string &key) const throw() {
    map_t::const_iterator it(properties_.find(key));
    return (it != properties_.end());
}

const std::string &sdpa::util::Properties::get(const std::string &key) const throw(PropertyLookupFailed) {
    map_t::const_iterator it(properties_.find(key));
    if (it != properties_.end()) {
        return it->second;
    } else {
        throw PropertyLookupFailed(key);
    }
}

const std::string &sdpa::util::Properties::get(const std::string &key, const std::string &def) const throw() {
    try {
        return get(key);
    } catch(...) {
        return def;
    }
}

void sdpa::util::Properties::clear() {
    properties_.clear();
}

bool sdpa::util::Properties::empty() const {
    return properties_.empty();
}

void sdpa::util::Properties::writeTo(std::ostream &os) const
{
  os << "[";
  map_t::const_iterator it(properties_.begin());
  for (;;)
  {
    os << "{"
       << it->first
       << ","
       << it->second
       << "}";

    ++it;
    if (it == properties_.end())
    {
      break;
    }
    else
    {
      os << ",";
    }
  }
  os << "]";
}

std::ostream &operator<<(std::ostream &os, const sdpa::util::Properties &props)
{
  props.writeTo(os);
  return os;
}
