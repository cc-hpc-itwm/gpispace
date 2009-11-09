/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// gwdl
#include <gwdl/Data.h>
#include <gwdl/XMLUtils.h>
//fhglog
#include <fhglog/fhglog.hpp>

using namespace std;
using namespace gwdl;

namespace gwdl
{

/**
 * The xmlstring MUST NOT contain the parent <data> element.
 */  
Data::Data(const string& xmlstring, const int type) throw(WorkflowFormatException) {
	_content = xmlstring;
	XMLUtils::trim(_content);
	LOG_DEBUG(logger_t(getLogger("gwdl")), "Data(" << _content << ")...");
	
	if ( _content.empty()) {
		_type = TYPE_EMPTY;
	} else {
		_type = type;
	}
}

Data::~Data() {
	LOG_DEBUG(logger_t(getLogger("gwdl")), "~Data()");
}

Data::ptr_t Data::deepCopy() {
	Data::ptr_t data(new Data(_content));
	LOG_DEBUG(logger_t(getLogger("gwdl")), "Data::deepCopy()");
	return data;
}

} // end namespace gwdl
