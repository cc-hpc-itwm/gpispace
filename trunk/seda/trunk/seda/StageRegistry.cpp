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

#include "StageRegistry.hpp"

using namespace seda;

StageRegistry::StageRegistry() : SEDA_INIT_LOGGER("seda.stage-registry") {}
StageRegistry::~StageRegistry() {}

StageRegistry& StageRegistry::instance() {
    static StageRegistry _instance;
    return _instance;
}

void StageRegistry::insert(const std::string& name, const Stage::Ptr& stage) throw(StageAlreadyRegistered) {
    // lookup the stage first
    try {
        lookup(name);
        throw StageAlreadyRegistered(name);
    } catch (const StageNotFound &) {
        _stages.insert(std::make_pair(name, stage));
        SEDA_LOG_DEBUG("added stage `" << name << "'");
    }
}
void StageRegistry::insert(const std::string& name, Stage* stage) throw(StageAlreadyRegistered) {
    insert(name, Stage::Ptr(stage));
}

void StageRegistry::insert(const Stage::Ptr& stage) throw(StageAlreadyRegistered) {
    insert(stage->name(), stage);
}
void StageRegistry::insert(Stage* stage) throw(StageAlreadyRegistered) {
    insert(stage->name(), Stage::Ptr(stage));
}

bool StageRegistry::remove(const Stage::Ptr &stage) {
    return remove(stage->name());
}

bool StageRegistry::remove(const std::string &name) {
    stage_map_t::iterator it(_stages.find(name));
    if (it == _stages.end()) {
        return false;
    } else {
        _stages.erase(it);
        return true;
    }
}

const Stage::Ptr StageRegistry::lookup(const std::string& name) const throw (StageNotFound) {
    stage_map_t::const_iterator it(_stages.find(name));
    if (it == _stages.end()) {
        throw StageNotFound(name);
    } else {
        return it->second;
    }
}

void StageRegistry::startAll() {
    stage_map_t::iterator it(_stages.begin());
    while (it != _stages.end()) {
        it->second->start(); it++;
    }
}

void StageRegistry::stopAll() {
    stage_map_t::iterator it(_stages.begin());
    while (it != _stages.end()) {
        it->second->stop(); it++;
    }
}

void StageRegistry::clear() {
    SEDA_LOG_DEBUG("removing all registered stages");
    _stages.clear();
}
