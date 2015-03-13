#pragma once

#include <fhglog/LogMacros.hpp>

#define LOG(level, msg) LLOG (level, ::fhg::log::GLOBAL_logger(), msg)
