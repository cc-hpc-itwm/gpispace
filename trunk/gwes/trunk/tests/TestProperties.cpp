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
#include "../gworkflowdl_cpp/src/XMLUtils.h"
//tests
#include "TestProperties.h"

using namespace std;
using namespace gwdl;
XERCES_CPP_NAMESPACE_USE

void testProperties()
{
	cout << "============== BEGIN PROPERTIES TEST =============" << endl;
	
	cout << "test put properties..." << endl;
	Properties *props = new Properties();
	props->put("key1","value1");
	props->put("key2","value2");
	props->put("key3","value3");
	
	cout << *props << endl;
	
	cout << "key1=" << props->get("key1") << endl;
	assert(props->get("key1")=="value1");
	cout << "key2=" << props->get("key2") << endl;
	assert(props->get("key2")=="value2");
	cout << "key3=" << props->get("key3") << endl;
	assert(props->get("key3")=="value3");
	assert(props->size()==3);
	
	cout << "test remove properties..." << endl;
	props->remove("key1");
    cout << "number of properties: " << props->size() << endl;
    assert(props->size()==2);
	
	cout << "test properties toElements() ..." << endl;
	vector<DOMElement*> elements = props->toElements(XMLUtils::Instance()->createEmptyDocument(true));
	cout << "number of elements: " << elements.size() << endl;
	cout << *props << endl;
	
	cout << "test overwriting of properties ..." << endl;
	props->put("key3","value3b");
	cout << *props << endl;
	assert(props->get("key3")=="value3b");

	delete props;

	cout << "============== END PROPERTIES TEST =============" << endl;
}
