#include <we/loader/macros.hpp>

#include <stdexcept>

WE_MOD_INITIALIZE_START()
{
  throw std::runtime_error ("initialize_throws");
}
WE_MOD_INITIALIZE_END()
