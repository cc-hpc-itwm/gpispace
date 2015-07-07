#include <we/loader/wrapper.hpp>

#include <stdexcept>

WE_MOD_INITIALIZE_START (initialize_throws);
{
  throw std::runtime_error ("initialize_throws");
}
WE_MOD_INITIALIZE_END (initialize_throws);
