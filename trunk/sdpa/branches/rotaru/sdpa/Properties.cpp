#include "Properties.hpp"

void sdpa::Properties::put(const std::string &key, const std::string &val) {
    del(key);
    _properties.insert(std::make_pair(key, val));
}

std::size_t sdpa::Properties::del(const std::string &key) {
    return _properties.erase(key);
}

bool sdpa::Properties::has_key(const std::string &key) const {
    std::map<std::string, std::string>::const_iterator it(_properties.find(key));
    return (it != _properties.end());
}

const std::string &sdpa::Properties::get(const std::string &key) const throw(PropertyLookupFailed) {
    std::map<std::string, std::string>::const_iterator it(_properties.find(key));
    if (it != _properties.end()) {
        return it->second;
    } else {
        throw PropertyLookupFailed(key);
    }
}

const std::string &sdpa::Properties::get(const std::string &key, const std::string &def) const throw() {
    try {
        return get(key);
    } catch(...) {
        return def;
    }
}

void sdpa::Properties::clear() {
    _properties.clear();
}

bool sdpa::Properties::empty() const {
    return _properties.empty();
}
