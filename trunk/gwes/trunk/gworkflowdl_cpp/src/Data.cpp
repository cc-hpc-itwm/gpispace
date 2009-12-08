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

Data::Data(const string& xmlstring, const int type) throw(WorkflowFormatException) {
	_content = xmlstring;
	XMLUtils::trim(_content);
	LOG_DEBUG(logger_t(getLogger("gwdl")), "Data(" << _content << ")...");
	
	if ( _content.empty()) {
		_type = TYPE_EMPTY;
	} else if ( XMLUtils::startsWith(_content,"<data") && XMLUtils::endsWith(_content,"</data>") ) {
		_type = type;
	} else {
		ostringstream oss; 
		oss << "XML for data object does not have parent element <data>! " << _content;
		LOG_ERROR(logger_t(getLogger("gwdl")), oss.str());
		throw WorkflowFormatException(oss.str());
	}
}

Data::~Data() {}

Data::ptr_t Data::deepCopy() {
	Data::ptr_t data(new Data(_content));
	return data;
}

} // end namespace gwdl

ostream& operator<<(ostream &out, gwdl::Data &data) {
	out << data.getContent();
	return out;
}
