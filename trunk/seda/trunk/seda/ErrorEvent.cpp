#include "ErrorEvent.hpp"

#include <stdexcept>

using namespace seda;

ErrorEvent::ErrorEvent(const std::exception& ex)
    : _ex(ex) {}

ErrorEvent::ErrorEvent(const std::string& reason)
    : _ex(std::runtime_error(reason)) {}

const std::exception& ErrorEvent::error() const {
    return _ex;
}
