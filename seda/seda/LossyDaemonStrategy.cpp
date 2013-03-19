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

#include "LossyDaemonStrategy.hpp"

#include <limits>

using namespace seda;

LossyDaemonStrategy::LossyDaemonStrategy(const Strategy::Ptr &s, double probability, unsigned int seed)
  : StrategyDecorator(s->name()+".lossy-daemon", s),
    probability_(probability),
    random_(generator_type(seed))
{
}

void
LossyDaemonStrategy::perform(const IEvent::Ptr &evt) {
    if ( random_() < probability_) {
      MLOG (TRACE, "lossy-daemon lost event: " << evt->str());
    } else {
        StrategyDecorator::perform(evt);
    }
}
