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

#ifndef SEDA_EVENT_DECORATOR_HPP
#define SEDA_EVENT_DECORATOR_HPP 1

#include <seda/IEvent.hpp>

namespace seda {
  class EventDecorator : public IEvent {
  public:
    EventDecorator(IEvent *e) : event(e) {}
    virtual ~EventDecorator() {
      if (event) {
	delete event; event = 0;
      }
    }

    virtual void visit(EventQueue* queue) {
      event->visit(queue);
    }

    virtual std::string str() const {
      return event->str();
    }
  private:
    IEvent* event;
  };
}

#endif // !SEDA_EVENT_DECORATOR_HPP
