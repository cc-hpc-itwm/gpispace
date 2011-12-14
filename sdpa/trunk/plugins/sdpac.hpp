#ifndef SDPA_PLUGIN_SDPAC_HPP
#define SDPA_PLUGIN_SDPAC_HPP 1

#include <string>

namespace sdpa
{
  namespace status
  {
    enum code
      {
        // non-terminal states
        UNKNOWN,
        PENDING,
        SUSPENDED,
        RUNNING,
        // terminal states
        FINISHED,
        FAILED,
        CANCELED,
      };
  }

  class Client
  {
  public:
    virtual ~Client() {}

    virtual int submit (std::string const &wf, std::string & job) = 0;
    virtual int execute (std::string const &wf, std::string & out) = 0;
    virtual int status (std::string const &id) = 0;
    virtual int cancel (std::string const &id) = 0;
    virtual int result (std::string const &id, std::string &out) = 0;
    virtual int remove (std::string const &id) = 0;
  };
}

#endif
