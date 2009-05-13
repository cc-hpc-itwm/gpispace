#include "ErrorEvent.hpp"

using namespace sdpa;

ErrorEvent::ErrorEvent(const std::string& reason, const std::string& additionalData)
  : _reason(reason), _additionalData(additionalData) {}

ErrorEvent::~ErrorEvent() {}

std::string ErrorEvent::str() const {
  if (additionalData().size()) {
    return reason() + ": " + additionalData();
  } else {
    return reason();
  }
}

const std::string& ErrorEvent::reason() const {
  return _reason;
}

const std::string& ErrorEvent::additionalData() const {
  return _additionalData;
}
