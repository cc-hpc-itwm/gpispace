#ifndef WFE_PLUGIN_TASK_HPP
#define WFE_PLUGIN_TASK_HPP 1

#include <string>
#include <map>

#include <we/we.hpp>
#include <fhg/plugin/capability.hpp>
#include <fhg/util/thread/event.hpp>

struct wfe_task_t
{
  typedef std::map<std::string, fhg::plugin::Capability*> capabilities_t;

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
  we::activity_t activity;
  capabilities_t capabilities;
  std::string result;
  fhg::util::thread::event<int> done;
};

#endif
