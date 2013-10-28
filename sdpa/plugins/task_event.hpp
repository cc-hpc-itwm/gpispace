#ifndef SDPA_PLUGIN_TASK_EVENT_HPP
#define SDPA_PLUGIN_TASK_EVENT_HPP 1

#include <map>
#include <string>
#include <list>

#include <sdpa/daemon/NotificationEvent.hpp>

struct task_event_t
{
  typedef std::map<std::string, std::string> meta_data_t;

  task_event_t ( std::string const & _id
               , std::string const & _name
               , const sdpa::daemon::NotificationEvent::state_t _state
               , std::string const & _activity
               , meta_data_t const & _meta
               , std::list<std::string> const& _worker_list
               )
    : id(_id)
    , name(_name)
    , state(_state)
    , activity(_activity)
    , meta(_meta)
    , worker_list (_worker_list)
  {}

  std::string id;
  std::string name;
  sdpa::daemon::NotificationEvent::state_t state;

  std::string activity;
  meta_data_t meta;
  std::list<std::string> worker_list;
};

#endif
