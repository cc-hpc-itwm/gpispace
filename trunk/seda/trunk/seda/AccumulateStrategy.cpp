#include "IEvent.hpp"
#include "IEventQueue.hpp"
#include "AccumulateStrategy.hpp"
#include <string>
#include <sstream>


namespace seda {
    void AccumulateStrategy::perform(const IEvent::Ptr& e) {
        {
            boost::unique_lock<boost::recursive_mutex> lock(_mtx);
            _accumulator.push_back(e);
        }
        StrategyDecorator::perform(e);
    }

    std::string AccumulateStrategy::str() const {
        std::ostringstream ostream(std::ostringstream::out);
        seda::AccumulateStrategy::const_iterator it;
        for (it=begin(); it != end(); it++) {
            ostream <<(*it)->str() << std::endl;
        }
        return ostream.str();
    }

    bool AccumulateStrategy::checkSequence(const std::list<std::string>& expected) {
        bool retval=true;
        if (expected.size() == _accumulator.size()) {
            seda::AccumulateStrategy::iterator accIt(_accumulator.begin());
            std::list<std::string>::const_iterator expIt(expected.begin());
            while (accIt != _accumulator.end() && expIt != expected.end()) {
                //std::cout << "Comparing type " << *expIt << " with " << typeid(*accIt->get()).name() << std::endl;
                if (! ((*expIt) == typeid((*accIt->get())).name())){
                    retval=false;
                    break;
                }
                accIt++;
                expIt++;
            }
        } else { // different length of expected and accumulator
            retval=false;
        }
        return retval;
    }
}
