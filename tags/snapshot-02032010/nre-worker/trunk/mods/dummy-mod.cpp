#include <sdpa/modules/Macros.hpp>

#include <fhglog/fhglog.hpp>

using namespace sdpa::modules;

void dummy (data_t &params)
{
  const std::string input (params.at("input").token().data());

  MLOG(DEBUG, "input = " << input);
  if (input == "fail")
  {
    throw std::runtime_error("dummy function: user requested to fail");
  }

  params["output"].token().data("Hello World!");
  MLOG(DEBUG, "output = " << params["output"].token().data());
}

SDPA_MOD_INIT_START(dummy)
{
  SDPA_REGISTER_FUN_START(dummy);
    SDPA_ADD_INP("input", std::string);
    SDPA_ADD_OUT("output", std::string);
  SDPA_REGISTER_FUN_END(dummy);
}
SDPA_MOD_INIT_END(dummy)
