#ifndef SDPA_PLUGIN_TASK_EVENT_HPP
#define SDPA_PLUGIN_TASK_EVENT_HPP 1

#include <map>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>

struct task_event_t
{
  typedef boost::posix_time::ptime time_type;
  typedef std::map<std::string, std::string> meta_data_t;

  enum state_t
    {
      ENQUEUED
    , DEQUEUED
    , FINISHED
    , CANCELED
    , FAILED
    , UNKNOWN
    };

  task_event_t ( std::string const & _id
               , std::string const & _name
               , const state_t _state
               , std::string const & _activity
               , meta_data_t const & _meta
               )
    : id(_id)
    , name(_name)
    , state(_state)
    , activity(_activity)
    , meta(_meta)
    , tstamp (boost::posix_time::microsec_clock::universal_time())
  {}

  std::string id;
  std::string name;
  state_t state;

  std::string activity;
  meta_data_t meta;

  time_type tstamp;
};

#endif
