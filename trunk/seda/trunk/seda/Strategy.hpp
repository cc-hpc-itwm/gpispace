#ifndef SEDA_STRATEGY_HPP
#define SEDA_STRATEGY_HPP 1

#include <seda/common.hpp>
#include <seda/shared_ptr.hpp>
#include <seda/EventNotSupported.hpp>

#include <seda/IEvent.hpp>

namespace seda {
    class IEvent;
  
    class Strategy {
    public:
        typedef std::tr1::shared_ptr<Strategy> Ptr;
    
        virtual ~Strategy() {}
        virtual void perform(const IEvent::Ptr& e) = 0; //{ throw seda::EventNotSupported(e); }
        virtual void onStageStart(const std::string& stageName) {}
        virtual void onStageStop(const std::string& stageName)  {}
        const std::string& name() const { return _name; }

        virtual std::string str() const { return name(); }
        /* TODO:  introduce a notation  for maximum  number of  threads this
           strategy supports.  It may be that particular  strategies must be
           executed sequentially. Can also be solved by acquiring a mutex from
           withing the perform method. */
    protected:
        explicit
        Strategy(const std::string& name) 
            : SEDA_INIT_LOGGER(name), _name(name)
        {}
        SEDA_DECLARE_LOGGER();
    private:
        std::string _name;
    };
}

#endif // ! SEDA_STRATEGY_HPP
