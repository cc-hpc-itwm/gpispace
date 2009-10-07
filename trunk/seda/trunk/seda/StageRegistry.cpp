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
    std::map<std::string, Stage::Ptr>::iterator it(_stages.find(name));
    if (it == _stages.end()) {
        return false;
    } else {
        _stages.erase(it);
        return true;
    }        
}

const Stage::Ptr StageRegistry::lookup(const std::string& name) const throw (StageNotFound) {
    std::map<std::string, Stage::Ptr>::const_iterator it(_stages.find(name));
    if (it == _stages.end()) {
        throw StageNotFound(name);
    } else {
        return it->second;
    }        
}

void StageRegistry::startAll() {
    std::map<std::string, Stage::Ptr>::iterator it(_stages.begin());
    while (it != _stages.end()) {
        it->second->start(); it++;
    }
}

void StageRegistry::stopAll() {
    std::map<std::string, Stage::Ptr>::iterator it(_stages.begin());
    while (it != _stages.end()) {
        it->second->stop(); it++;
    }
}

void StageRegistry::clear() {
    SEDA_LOG_DEBUG("removing all registered stages");
    _stages.clear();
}
