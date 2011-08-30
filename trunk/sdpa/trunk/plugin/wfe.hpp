#ifndef WFE_PLUGIN_HPP
#define WFE_PLUGIN_HPP 1

#include <map>
#include <fhg/plugin/capability.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

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
                        , boost::posix_time::time_duration = boost::posix_time::seconds(0)
                        ) = 0;
    virtual int cancel (std::string const &job_id) = 0;
  };
}

#endif
