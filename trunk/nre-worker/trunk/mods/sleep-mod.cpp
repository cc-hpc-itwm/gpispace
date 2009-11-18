#include <sdpa/modules/Macros.hpp>

#include <fhglog/fhglog.hpp>

#include <unistd.h>

using namespace sdpa::modules;

void sleep (data_t &params)
{
  const useconds_t usec = (params.at("usec").token().data_as<useconds_t>());

  LOG(INFO, "going to sleep for " << usec << " micro seconds");
  int retval = usleep(usec);
  LOG(INFO, "woke up: " << retval);

  params["error_code"].token().data(retval);
}

SDPA_MOD_INIT_START(sleep)
{
  SDPA_REGISTER_FUN_START(sleep);
    SDPA_ADD_INP("usec", useconds_t );
    SDPA_ADD_OUT("error_code", int);
  SDPA_REGISTER_FUN_END(sleep);
}
SDPA_MOD_INIT_END(sleep)
