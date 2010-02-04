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
        typedef seda::shared_ptr<Strategy> Ptr;
    
        virtual ~Strategy() {}
        virtual void perform(const IEvent::Ptr&) = 0; //{ throw seda::EventNotSupported(e); }
        virtual void onStageStart(const std::string&) {}
        virtual void onStageStop(const std::string&)  {}
        const std::string& name() const { return _name; }

        virtual std::string str() const { return name(); }
        /* TODO:  introduce a notation  for maximum  number of  threads this
           strategy supports.  It may be that particular  strategies must be
           executed sequentially. Can also be solved by acquiring a mutex from
           withing the perform method. */
    protected:
        explicit
        Strategy(const std::string& a_name) 
            : SEDA_INIT_LOGGER(a_name), _name(a_name)
        {}
        SEDA_DECLARE_LOGGER();
    private:
        std::string _name;
    };
}

#endif // ! SEDA_STRATEGY_HPP
