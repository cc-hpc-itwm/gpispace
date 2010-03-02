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

#ifndef SEDA_STAGE_FACTORY_HPP
#define SEDA_STAGE_FACTORY_HPP 1

#include <seda/Stage.hpp>
#include <seda/StageRegistry.hpp>

namespace seda {

    class StageFactory {
        public:
            typedef seda::shared_ptr<StageFactory> Ptr;

            explicit
            StageFactory() {}
            virtual ~StageFactory() {}

            virtual seda::Stage::Ptr createStage(const std::string &name,
                    seda::Strategy::Ptr strategy,
                    std::size_t maxPoolSize=1,
                    std::size_t maxQueueSize=SEDA_MAX_QUEUE_SIZE,
                    const std::string &errorHandler="system-event-handler") const {
                seda::Stage::Ptr stage(new seda::Stage(name, strategy, maxPoolSize, maxQueueSize, errorHandler));
                StageRegistry::instance().insert(stage);
                return stage;
            }
    };
}

#endif // SEDA_STAGE_FACTORY_HPP
