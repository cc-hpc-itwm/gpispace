#ifndef SDPA_NODE_HPP
#define SDPA_NODE_HPP 1

#include <list>
#include <string>
#include <sdpa/util.hpp>
#include <sdpa/Job.hpp>
#include <sdpa/events/SDPAEvent.hpp>

namespace sdpa {
  // TODO: to be replaced by a real class (synchronization!)
  typedef std::list<Job::ptr_t> JobQueue;

  /**
    This class holds all information about an attached node.

    On the orchestrator this represents an aggregator and on the aggregator
    all information about attached NREs is held in this class.
  */
  class Node {
  public:
    Node(const std::string &name, const std::string &location);

    void update(const sdpa::events::SDPAEvent &event);
    void dispatch(Job::ptr_t &job);

    sdpa::util::time_type tstamp() const { return tstamp_; }
    const std::string &name() const { return name_; }
    const std::string &location() const { return location_; }
  private:
    std::string name_; //! name of the node
    std::string location_; //! location where to reach the node
    sdpa::util::time_type tstamp_; //! time of last message received

    JobQueue pending_queue_; //! the queue of jobs assigned to this node (not yet confirmed)
    JobQueue submitted_queue_; //! the queue of jobs assigned to this node (successfully submitted)
  };
}

#endif
