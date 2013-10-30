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

#ifndef SEDA_STAGE_WORKER_HPP
#define SEDA_STAGE_WORKER_HPP

#include <string>
#include <seda/common.hpp>

namespace seda {
    class Stage;
  
    class StageWorker {
    public:
        StageWorker(const std::string& id, Stage* s) :
            SEDA_INIT_LOGGER(id),
            _stage(s),
            _busy(false),
            _stopped(false)
        { }

        void stop() { _stopped = true; }
        void operator()() { run(); }
        void run();
        bool busy() const { return _busy; }

    private:
        SEDA_DECLARE_LOGGER();
        bool stopped() { return _stopped; }
      
        Stage* _stage;
        bool _busy;
        bool _stopped;
    };
}

#endif // !SEDA_STAGE_WORKER_HPP
