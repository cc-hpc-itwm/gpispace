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

#ifndef SEDA_TIMER_HPP
#define SEDA_TIMER_HPP 1

#include <boost/thread.hpp>
#include <seda/shared_ptr.hpp>

namespace seda {
    class Timer {
        public:
            typedef seda::shared_ptr<Timer> Ptr;

            explicit
            Timer(const std::string &a_targetStage,
                  const boost::posix_time::time_duration &a_interval = boost::posix_time::seconds(1),
                  const std::string& a_tag = "tick")
                : _targetStage(a_targetStage), _interval(a_interval), _tag(a_tag), _active(false) {}
            virtual ~Timer() { stop(); }

            void start();
            void stop();

            const std::string& targetStage() const { return _targetStage; }
            const std::string& tag() const { return _tag; }
            std::string& tag() { return _tag; }

            const boost::posix_time::time_duration& interval() const { return _interval; }
            boost::posix_time::time_duration& interval() { return _interval; }

            // thread entry function
            void operator()();
        private:
            bool active();

            std::string _targetStage;
            boost::posix_time::time_duration _interval;
            std::string _tag;
            bool _active;
            boost::thread _thread;
            boost::recursive_mutex _mtx;
            boost::condition_variable_any _cond;
    };
}

#endif // ! SEDA_TIMER_HPP
