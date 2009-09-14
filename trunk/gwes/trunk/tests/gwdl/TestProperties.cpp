/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#include <iostream>
#include <ostream>
#include <assert.h>
//gwdl
#include <gwdl/XMLUtils.h>
//tests
#include "TestProperties.h"
//fhglog
#include <fhglog/fhglog.hpp>

using namespace std;
using namespace gwdl;
using namespace fhg::log;
XERCES_CPP_NAMESPACE_USE

void testProperties()
{
	logger_t logger(logger_t(getLogger("gwdl")));
	LOG_INFO(logger, "============== BEGIN PROPERTIES TEST =============");
	
	LOG_INFO(logger, "test put properties...");
	Properties *props = new Properties();
	props->put("key1","value1");
	props->put("key2","value2");
	props->put("key3","value3");
	
	LOG_INFO(logger, *props);
	
	LOG_INFO(logger, "key1=" << props->get("key1"));
	assert(props->get("key1")=="value1");
	LOG_INFO(logger, "key2=" << props->get("key2"));
	assert(props->get("key2")=="value2");
	LOG_INFO(logger, "key3=" << props->get("key3"));
	assert(props->get("key3")=="value3");
	assert(props->size()==3);
	
	LOG_INFO(logger, "test remove properties...");
	props->remove("key1");
    LOG_INFO(logger, "number of properties: " << props->size());
    assert(props->size()==2);
	
	LOG_INFO(logger, "test properties toElements() ...");
	vector<DOMElement*> elements = props->toElements(XMLUtils::Instance()->createEmptyDocument(true));
	LOG_INFO(logger, "number of elements: " << elements.size());
	LOG_INFO(logger, *props);
	
	LOG_INFO(logger, "test overwriting of properties ...");
	props->put("key3","value3b");
	LOG_INFO(logger, *props);
	assert(props->get("key3")=="value3b");

	delete props;

	LOG_INFO(logger, "============== END PROPERTIES TEST =============");
}
