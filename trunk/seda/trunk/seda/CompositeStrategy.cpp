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

#include "CompositeStrategy.hpp"

using namespace seda;

CompositeStrategy::CompositeStrategy(const std::string& a_name)
  : Strategy(a_name) {}

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

void CompositeStrategy::onStageStart(const std::string& s)
{
  for (std::list<Strategy::Ptr>::const_iterator it(_children.begin());
       it != _children.end();
       it++) {
    (*it)->onStageStart(s);
  }
}

void CompositeStrategy::onStageStop(const std::string& s)
{
  for (std::list<Strategy::Ptr>::const_iterator it(_children.begin());
       it != _children.end();
       it++) {
    (*it)->onStageStop(s);
  }
}

void CompositeStrategy::perform(const IEvent::Ptr& e) {
  for (std::list<Strategy::Ptr>::const_iterator it(_children.begin());
       it != _children.end();
       it++) {
    (*it)->perform(e);
  }
}
