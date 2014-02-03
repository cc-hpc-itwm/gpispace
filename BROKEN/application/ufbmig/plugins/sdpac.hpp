#ifndef SDPA_PLUGIN_SDPAC_HPP
#define SDPA_PLUGIN_SDPAC_HPP 1

#include <string>
#include <sdpa/job_states.hpp>

namespace sdpa
{
  class Client
  {
  public:
    virtual ~Client() {}

    virtual int submit (std::string const &wf, std::string & job) = 0;
    virtual int execute (std::string const &wf, std::string & out) = 0;
    virtual int status (std::string const &id, int & ec, std::string & msg) = 0;
    virtual int cancel (std::string const &id) = 0;
    virtual int result (std::string const &id, std::string &out) = 0;
    virtual int remove (std::string const &id) = 0;
    virtual int unload_modules () = 0;
  };
}

#endif
