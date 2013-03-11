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

#ifndef SEDA_LOSSY_DAEMON_STRATEGY_HPP
#define SEDA_LOSSY_DAEMON_STRATEGY_HPP 1

#include <iostream>
#include <boost/random.hpp>
#include <seda/StrategyDecorator.hpp>

namespace seda {
    class LossyDaemonStrategy : public StrategyDecorator {
        public:
            typedef shared_ptr<LossyDaemonStrategy> Ptr;

            explicit
            LossyDaemonStrategy(const Strategy::Ptr &s, double probability=0.1, unsigned int seed=1);
            ~LossyDaemonStrategy() {}

            void set_probability (double p)
            {
              probability_ = p;
            }

            double get_probability (void) const
            {
              return probability_;
            }

            void perform(const IEvent::Ptr&);
        private:
            double probability_;
      //            unsigned int seed_;

            typedef boost::minstd_rand generator_type;
            typedef boost::uniform_01<generator_type> gen_type;
            gen_type random_;
    };
}

#endif // !SEDA_LOSSY_DAEMON_STRATEGY_HPP
