#ifndef SDPA_CLIENT_JOB_STATUS_HPP
#define SDPA_CLIENT_JOB_STATUS_HPP

#include <string>

namespace sdpa
{
  namespace client
  {
    struct job_info_t
    {
      int error_code;
      std::string error_message;
    };
  }
}

#endif
