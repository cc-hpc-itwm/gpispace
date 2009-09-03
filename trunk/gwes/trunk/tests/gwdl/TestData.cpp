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
#include "TestData.h"

using namespace std;
using namespace gwdl;
XERCES_CPP_NAMESPACE_USE

#define X(str) XMLString::transcode((const char*)& str)

void testData()
{
    cout << "============== BEGIN DATA TEST =============" << endl;
	
	cout << "test empty data token... " << endl;
	Data *emptyData = new Data();
	assert(emptyData->toElement(NULL)==NULL);
	delete emptyData;
 
	cout << "test xml data token from DOM element... " << endl;
	DOMImplementation* impl (DOMImplementationRegistry::getDOMImplementation (X("LS")));
	DOMDocument* doc = impl->createDocument(0,X("workflow"),0);
	// <data>
	DOMElement* dataElement = doc->createElement(X("data"));
	// <value1>25</value1>
	DOMElement* e1 = doc->createElement(X("value1"));
	dataElement->appendChild(e1);
	DOMText* e1value = doc->createTextNode(X("25"));
	e1->appendChild(e1value);
	// <value2>15</value1>
	DOMElement* e2 = doc->createElement(X("value2"));
	dataElement->appendChild(e2);
	DOMText* e2value = doc->createTextNode(X("15"));
	e2->appendChild(e2value);
	
	Data *data = new Data(dataElement);
	cout << "test data << operator ..." << endl;
	cout << *data << endl;
	cout << "test data.toString() ..." << endl;
	string* datastr = data->toString();
	cout << *datastr << endl;
	assert(*(data->toString())=="<data><value1>25</value1><value2>15</value2></data>");
	delete data;
	
	cout << "test constructing data from string..." << endl;
	string *str = new string("<data xmlns=\"http://www.gridworkflow.org/gworkflowdl\"><x>1</x><y>2</y></data>");
	Data *data2 = new Data(*str);
	cout << *data2 << endl;
	cout << *str << endl;
	char* name = XMLString::transcode(data->toElement(NULL)->getTagName()); 
	assert(!strcmp(name,"data"));
	
	///ToDo: FAILED TO DELETE data2!
	//delete data2;
	
	///ToDo: FAILED WITH INTEL COMPILER/// 
	cout << "test workflow format exception ..." << endl;
	string *str3 = new string("<bla>");
	bool test = false;
	try {
		Data *data3 = new Data(*str3);
        cout << *data3 << endl;
    } catch(WorkflowFormatException e) {
   	    cout << "WorkflowFormatException: " << e.message << endl;
   	    test = true;
    } catch (...) {
        cout << "another exception" << endl;
	}
    assert(test);
    delete str3;
	
    cout << "============== END DATA TEST =============" << endl;
	
}
