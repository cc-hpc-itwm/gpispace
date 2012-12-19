#ifndef WFE_PLUGIN_TASK_HPP
#define WFE_PLUGIN_TASK_HPP 1

#include <string>
#include <map>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <we/mgmt/type/activity.hpp>
#include <fhg/plugin/capability.hpp>
#include <fhg/util/thread/event.hpp>

struct wfe_task_t
{
  typedef boost::posix_time::ptime time_type;
  typedef std::map<std::string, fhg::plugin::Capability*> capabilities_t;
  typedef std::map<std::string, std::string> meta_data_t;

  enum state_t
    {
      PENDING
    , CANCELED
    , FINISHED
    , FAILED
    };

  std::string id;
  int        state;
  int        errc;
  we::mgmt::type::activity_t activity;
  capabilities_t capabilities;
  std::string result;
  fhg::util::thread::event<int> done;
  meta_data_t meta;
  std::string error_message;

  time_type enqueue_time;
  time_type dequeue_time;
  time_type finished_time;
};

#endif
