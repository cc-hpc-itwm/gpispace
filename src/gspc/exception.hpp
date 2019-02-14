#pragma once

#include <boost/format.hpp>

#include <exception>
#include <stdexcept>

#define SHOW(what...) (boost::format ("%1%") % (what)).str()

#define INVALID_ARGUMENT(what...)                               \
  throw std::invalid_argument (SHOW (what))

#define LOGIC_ERROR(what...)                                    \
  throw std::logic_error (SHOW (what))

#define INCONSISTENCY(what...)                                  \
  LOGIC_ERROR                                                   \
    (SHOW (boost::format ("INCONSISTENCY: %1%") % (what)))

#define NESTED_RUNTIME_ERROR(what...)                           \
  std::throw_with_nested (std::runtime_error (SHOW (what)))
