/* 
   Copyright (C) 2009 Alexander Petry <alexander.petry@itwm.fraunhofer.de>.

   This file is part of seda.

   seda is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   seda is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with seda; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  

*/

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
