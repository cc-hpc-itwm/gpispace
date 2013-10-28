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

#ifndef SEDA_ACCUMULATESTRATEGY_HPP
#define SEDA_ACCUMULATESTRATEGY_HPP 1

#include <seda/StrategyDecorator.hpp>
#include <boost/thread.hpp>
#include <list>
#include <typeinfo>



namespace seda {
  /* Accumulates the events sent to this StrategyDecorator.
   */
  class AccumulateStrategy : public StrategyDecorator {
    public:
      typedef std::list<IEvent::Ptr>::iterator iterator;
      typedef std::list<IEvent::Ptr>::const_iterator const_iterator;
      typedef seda::shared_ptr<AccumulateStrategy> Ptr;
      explicit AccumulateStrategy(const Strategy::Ptr& s)
                : StrategyDecorator(s->name()+".accumulate", s),
        _accumulator() {}

      void perform(const IEvent::Ptr&);
      std::size_t size() { return _accumulator.size(); }
      void clear() { _accumulator.clear(); }
      bool empty() { return (_accumulator.size() == 0); }

      iterator begin() { return _accumulator.begin(); }
      iterator end() { return _accumulator.end(); }

      const_iterator begin() const { return _accumulator.begin(); }
      const_iterator end() const { return _accumulator.end(); }
      std::string str() const;

      /***
       * Provide a list of type-names that represent the type
       * information. Example:
       * std::list<std::string> expectedSequence;
       * expectedSequence.push_back(typeid(StringEvent).name());
       * CPPUNIT_ASSERT_EQUAL(true, _accumulate->checkSequence(expectedSequence));
       */
      bool checkSequence(const std::list<std::string>& );

    private:
      std::list<IEvent::Ptr> _accumulator;
      boost::recursive_mutex _mtx;
  };
}

#endif /* SEDA_ACCUMULATESTRATEGY_HPP */
