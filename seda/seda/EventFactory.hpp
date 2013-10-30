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

#ifndef XBE_EVENT_FACTORY_HPP
#define XBE_EVENT_FACTORY_HPP 1

#include <cms/Message.h>
#include <cms/TextMessage.h>
#include <cms/CMSException.h>

#include <seda/IEvent.hpp>

#include <xbe/XbeException.hpp>

namespace xbe {
  class EventFactoryException : public XbeException {
  public:
    explicit
    EventFactoryException(const std::string& reason)
      : XbeException(reason) {};
  };
  class UnknownConversion : public EventFactoryException {
  public:
    explicit
    UnknownConversion(const std::string& reason)
      : EventFactoryException(reason) {};
  };
  
  class EventFactory {
  public:
    static const EventFactory& instance();

    seda::IEvent::Ptr newEvent(const cms::Message*) const throw(UnknownConversion);
    seda::IEvent::Ptr newEvent(const cms::TextMessage*) const;
    seda::IEvent::Ptr newEvent(const cms::CMSException&) const;

  private:
    EventFactory();
  };
}

#endif // !XBE_EVENT_FACTORY_HPP
