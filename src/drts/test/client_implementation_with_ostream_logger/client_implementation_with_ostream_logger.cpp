#include "client_implementation_with_ostream_logger.hpp"

NO_NAME_MANGLING void client_with_ostream_logger
  (std::string const& message, std::ostream& os)
{
  os << message;
}
