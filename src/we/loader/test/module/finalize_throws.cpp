#include <we/loader/macros.hpp>

#include <stdexcept>

WE_MOD_INITIALIZE_START (finalize_throws);
{
}
WE_MOD_INITIALIZE_END (finalize_throws);

WE_MOD_FINALIZE_START (finalize_throws);
{
  throw std::runtime_error ("finalize_throws");
}
WE_MOD_FINALIZE_END (finalize_throws);
