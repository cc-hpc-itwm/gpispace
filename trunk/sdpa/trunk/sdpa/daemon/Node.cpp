#include "Node.hpp"

using namespace sdpa;

Node::Node(const std::string &name, const std::string &location)
  : name_(name), location_(location), tstamp_(sdpa::util::now()) {}

void Node::update(const sdpa::events::SDPAEvent &event) {
  tstamp_ = sdpa::util::now();
}

void Node::dispatch(Job::ptr_t &job) {
  pending_queue_.push_back(job);
}
