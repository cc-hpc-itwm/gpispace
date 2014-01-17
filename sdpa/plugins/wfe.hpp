#ifndef WFE_PLUGIN_HPP
#define WFE_PLUGIN_HPP 1

#include <map>
#include <list>
#include <fhg/plugin/capability.hpp>
#include <string>

namespace wfe
{
  typedef std::map<std::string, fhg::plugin::Capability*> capabilities_t;

  class WFE
  {
  public:
    virtual ~WFE() {}

    virtual int execute ( std::string const &job_id
                        , std::string const &job_description
                        , capabilities_t const & capabilities
                        , std::string & result
                        , std::string & error_message
                        , std::list<std::string> const & worker_list
                        ) = 0;
    virtual int cancel (std::string const &job_id) = 0;
  };
}

#endif
