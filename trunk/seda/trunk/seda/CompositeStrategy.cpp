#include "CompositeStrategy.hpp"

using namespace seda;

CompositeStrategy::CompositeStrategy(const std::string& name)
  : Strategy(name) {}

void CompositeStrategy::add(const Strategy::Ptr& s) {
  _children.push_back(s);
}

void CompositeStrategy::remove(const Strategy::Ptr& s) {
  for (std::list<Strategy::Ptr>::iterator it(_children.begin());
       it != _children.end();
       it++) {
    if (*it == s) {
      _children.erase(it); break;
    }
  }
}

void CompositeStrategy::perform(const IEvent::Ptr& e) {
  for (std::list<Strategy::Ptr>::const_iterator it(_children.begin());
       it != _children.end();
       it++) {
    (*it)->perform(e);
  }
}
