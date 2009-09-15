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
//fhglog
#include <fhglog/fhglog.hpp>

using namespace std;
using namespace gwdl;
using namespace fhg::log;
XERCES_CPP_NAMESPACE_USE

#define X(str) XMLString::transcode((const char*)& str)
#define D(str) XMLString::release(&str)
#define SAFE_DELETE(ptr) if ((ptr) != 0) { delete (ptr); ptr=0; }

void testData()
{
	logger_t logger(Logger::get("gwdl"));
    LOG_INFO(logger, "============== BEGIN DATA TEST =============");
	
	LOG_INFO(logger, "test empty data token... ");
    {
      Data emptyData;
      assert(emptyData.toElement(NULL)==NULL);
    }
 
	LOG_INFO(logger, "test xml data token from DOM element... ");
    {
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
      
      Data data(dataElement);
      LOG_INFO(logger, "test data << operator ...");
      LOG_DEBUG(logger, data);
      LOG_INFO(logger, "test data.toString() ...");
      string* datastr = data.toString();
      LOG_INFO(logger, *datastr);
      assert((*datastr)=="<data><value1>25</value1><value2>15</value2></data>");
      SAFE_DELETE(datastr);
    }

	LOG_INFO(logger, "test xml data token (loop) from DOM element... ");
    {
      DOMImplementation* impl (DOMImplementationRegistry::getDOMImplementation (X("LS")));
      // who releases this doc?
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
      
      std::list<Data> dataList;
      for (std::size_t numElements(0); numElements < 100000; ++numElements)
      {
        Data data(dataElement);
        string* datastr = data.toString();
        assert((*datastr)=="<data><value1>25</value1><value2>15</value2></data>");
        SAFE_DELETE(datastr);
        dataList.push_back(data);
      }
    }
	
	LOG_INFO(logger, "test constructing data from string...");
    // FIXME: the problem is in the destructor of Data, it ->release() the
    // internal DOMElement structure which ends up  in a delete 0;
    {
      string str("<data xmlns=\"http://www.gridworkflow.org/gworkflowdl\"><x>1</x><y>2</y></data>");
      Data data2(str);
      LOG_INFO(logger, data2);
      LOG_INFO(logger, str);
      DOMElement* eP = data2.toElement(NULL);
      assert(eP);
      char* name = XMLString::transcode(eP->getTagName()); 
      assert(strcmp(name,"data")==0);
      D(name);
//      delete [] name; // or free()?
//      SAFE_DELETE(eP); // this fails!
	}


	///ToDo: FAILED TO DELETE data2!
	//delete data2;
	
	///ToDo: FAILED WITH INTEL COMPILER/// 
	LOG_INFO(logger, "test workflow format exception ...");
	string str3("<bla>");
	bool test = false;
	try {
		Data data3(str3);
        LOG_INFO(logger, data3);
    } catch(WorkflowFormatException e) {
   	    LOG_INFO(logger, "WorkflowFormatException: " << e.message);
   	    test = true;
    } catch (...) {
        LOG_INFO(logger, "another exception");
	}
    assert(test);
	
    LOG_INFO(logger, "============== END DATA TEST =============");
}
