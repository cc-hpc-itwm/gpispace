#ifndef SDPA_PLUGIN_TASK_EVENT_HPP
#define SDPA_PLUGIN_TASK_EVENT_HPP 1

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>

struct task_event_t
{
  typedef boost::posix_time::ptime time_type;

  enum state_t
    {
      ENQUEUED
    , DEQUEUED
    , FINISHED
    , CANCELED
    , FAILED
    };

  task_event_t ( std::string const & _id
               , std::string const & _name
               , const state_t _state
               , std::string const & _input = ""
               , std::string const & _output = ""
               )
    : id(_id)
    , name(_name)
    , state(_state)
    , input(_input)
    , output(_output)
  {
    time_type now = boost::posix_time::microsec_clock::universal_time();
    switch (state)
    {
    case ENQUEUED:
      enqueue_time = now;
      break;
    case DEQUEUED:
      dequeue_time = now;
      break;
    default:
      completion_time = now;
    }
  }

  std::string id;
  std::string name;
  state_t state;

  std::string input;
  std::string output;

  time_type enqueue_time;    // entered system
  time_type dequeue_time;    // removed from pending queue
  time_type completion_time;
};

#endif
