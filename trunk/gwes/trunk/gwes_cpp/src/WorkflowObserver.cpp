/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// gwes
#include <gwes/WorkflowObserver.h>

using namespace std;

namespace gwes
{

WorkflowObserver::WorkflowObserver() : _logger(fhg::log::Logger::get("gwes"))
{
}

WorkflowObserver::~WorkflowObserver()
{
}

void WorkflowObserver::update(const Event& event)
{
	LOG_DEBUG(_logger, "gwes::WorkflowObserver::update(" << event._sourceId << "," << event._eventType << "," << event._message << ")");
}

}
