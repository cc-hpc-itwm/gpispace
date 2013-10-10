#ifndef WFE_PLUGIN_HPP
#define WFE_PLUGIN_HPP 1

#include <map>
#include <list>
#include <fhg/plugin/capability.hpp>
#include <string>

namespace wfe
{
  typedef std::map<std::string, fhg::plugin::Capability*> capabilities_t;
  typedef std::map<std::string, std::string> meta_data_t;

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
                        , meta_data_t const &meta_data = meta_data_t()
                        ) = 0;
    virtual int cancel (std::string const &job_id) = 0;
  };
}

#endif
