#ifndef SEDA_COMPOSITE_STRATEGY_HPP
#define SEDA_COMPOSITE_STRATEGY_HPP 1

#include <seda/Strategy.hpp>
#include <list>

namespace seda {
  class CompositeStrategy : public seda::Strategy {
  public:
    typedef std::tr1::shared_ptr<CompositeStrategy> Ptr;
    
    CompositeStrategy(const std::string& name);
    ~CompositeStrategy() {}

    void add(const seda::Strategy::Ptr&);
    void remove(const seda::Strategy::Ptr&);

    void perform(const seda::IEvent::Ptr&);
  private:
    std::list<seda::Strategy::Ptr> _children;
  };
}

#endif // !SEDA_COMPOSITE_STRATEGY_HPP
