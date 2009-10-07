#ifndef SEDA_COUT_STRATEGY_HPP
#define SEDA_COUT_STRATEGY_HPP 1

#include <iostream>

#include <seda/Strategy.hpp>

namespace seda {
  class CoutStrategy : public Strategy {
  public:
    CoutStrategy(std::ostream& out=std::cout) : Strategy("cout"), _os(out) {}
    ~CoutStrategy() {}

    void perform(const IEvent::Ptr&);
  private:
    std::ostream& _os;
  };
}

#endif // !SEDA_COUT_STRATEGY_HPP
