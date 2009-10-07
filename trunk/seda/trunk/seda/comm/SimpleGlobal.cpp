#include "SimpleGlobal.hpp"

#include <iostream>

using namespace seda::comm;

SimpleGlobal::SimpleGlobal(const std::string &s) {
}

SimpleGlobal::~SimpleGlobal() {
}

void SimpleGlobal::put(const std::string &k, const std::string &v) {
    std::cout << "put: " << k << " -> " << v << std::endl;
}

std::string SimpleGlobal::get(const std::string &k) {
    std::cout << "get: " << k << std::endl;
    return "";
}
