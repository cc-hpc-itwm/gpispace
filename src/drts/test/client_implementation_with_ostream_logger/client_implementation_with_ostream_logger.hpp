#pragma once

#include <ostream>
#include <string>

#define NO_NAME_MANGLING extern "C"

NO_NAME_MANGLING void client_with_ostream_logger
  (std::string const&, std::ostream&);
