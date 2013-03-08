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

#ifndef SEDA_STAGEREGISTRY_HPP
#define SEDA_STAGEREGISTRY_HPP 1

#include <seda/Stage.hpp>
#include <seda/StageNotFound.hpp>
#include <seda/StageAlreadyRegistered.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <list>

namespace seda {
    /**
     * TODO: make this class thread-safe!
     */
    class StageRegistry {
    public:
        ~StageRegistry();

        static StageRegistry& instance();

        // access functions

        /**
         * Register a new stage with the supplied name.
         */
        void insert(const std::string& name, const Stage::Ptr& stage) throw(StageAlreadyRegistered);

        /**
         * Register a new stage and take over the ownership of it.
         */
        void insert(const std::string& name, Stage* stage) throw(StageAlreadyRegistered);

        /**
         * Register a new stage with the name of the stage.
         */
        void insert(const Stage::Ptr& stage) throw(StageAlreadyRegistered);

        /**
         * Register a new stage and take over the ownership of it.
         */
        void insert(Stage* stage) throw(StageAlreadyRegistered);

        /**
         * Lookup a stage by its name.
         */
        const Stage::Ptr lookup(const std::string& name) const throw(StageNotFound);

        /**
         * Remove a stage from the registry
         */
        bool remove(const Stage::Ptr &stage);

        /**
         * Remove a stage from the registry using its name.
         */
        bool remove(const std::string &name);

        /**
         * Collectively start all stages.
         */
        void startAll();

        /**
         * Collectively stop all stages.
         */
        void stopAll();

        /**
         * Remove all registered stages.
         */
        void clear();
    private:
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        typedef boost::unordered_map<std::string, Stage::Ptr> stage_map_t;
        typedef std::list<std::string> stage_names_t;

        StageRegistry();
        StageRegistry(const StageRegistry&);
        void operator=(const StageRegistry&);

        SEDA_DECLARE_LOGGER();
        stage_map_t _stages;
        stage_names_t _stage_names;

        mutable mutex_type m_mutex;
    };
}

#endif // ! SEDA_STAGEREGISTRY_HPP
