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

#ifndef SEDA_FORWARD_STRATEGY_HPP
#define SEDA_FORWARD_STRATEGY_HPP 1

#include <seda/Stage.hpp>
#include <seda/Strategy.hpp>

namespace seda {
    class ForwardStrategy : public Strategy {
    public:
        explicit
        ForwardStrategy(const std::string& a_next)
          : Strategy("fwd-to-"+a_next), _next(a_next) {}

        ForwardStrategy(const std::string& a_name, const std::string& a_next)
            : Strategy(a_name), _next(a_next) {}

        virtual void perform(const IEvent::Ptr&);

        virtual void next(const std::string& a_next) { _next = a_next; }
        virtual const std::string& next() const { return _next; }

      virtual void onStageStart(const std::string&);
      virtual void onStageStop(const std::string&);

    private:
        std::string _next;
      Stage::Ptr _next_stage;
    };
}

#endif // !SEDA_FORWARD_STRATEGY_HPP
