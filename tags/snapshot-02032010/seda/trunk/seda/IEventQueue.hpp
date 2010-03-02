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

#ifndef SEDA_IEVENT_QUEUE_HPP
#define SEDA_IEVENT_QUEUE_HPP 1

#include <boost/thread.hpp>

#include <seda/shared_ptr.hpp>
#include <seda/IEvent.hpp>
#include <seda/SedaException.hpp>

namespace seda {
  class QueueEmpty : public SedaException {
  public:
    QueueEmpty() : SedaException("queue is empty") {}
  };

  class QueueFull : public SedaException {
  public:
    QueueFull() : SedaException("queue is full") {}
  };

  class IEventQueue {
  public:
    typedef seda::shared_ptr<IEventQueue> Ptr;
    
    virtual ~IEventQueue() {}

    virtual IEvent::Ptr pop() throw (QueueEmpty, boost::thread_interrupted) = 0;
    virtual IEvent::Ptr pop(unsigned long millis) throw (QueueEmpty, boost::thread_interrupted) = 0;
    
    virtual void push(const IEvent::Ptr& e) throw (QueueFull) = 0;
    
    virtual void clear() = 0;

    virtual std::size_t size() const = 0;
    virtual bool empty() const = 0;
    virtual std::size_t maxQueueSize() const = 0;
    virtual void maxQueueSize(const std::size_t& max) = 0;

    virtual bool waitUntilEmpty() = 0;
    virtual bool waitUntilEmpty(unsigned long millis) = 0;
    virtual bool waitUntilNotEmpty() = 0;
    virtual bool waitUntilNotEmpty(unsigned long millis) = 0;

    virtual void wakeUpAll() = 0;
  protected:
    IEventQueue() {}
  };
}

#endif // !SEDA_IEVENT_QUEUE_HPP
