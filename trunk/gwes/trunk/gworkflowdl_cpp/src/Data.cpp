/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#include <iostream>
#include <sstream>
// gwdl
#include <gwdl/XMLUtils.h>
#include <gwdl/Data.h>

using namespace std;
using namespace gwdl;
XERCES_CPP_NAMESPACE_USE

#define X(str) XMLString::transcode((const char*)& str)
#define S(str) XMLString::transcode(str)

namespace gwdl
{

	Data::Data()
	{
	  data = NULL;
	  dataText = NULL;
	}
	
	Data::Data(DOMElement* element)
	{
	  data = element;
	  dataText = NULL;
	  setType();
	}
	
    Data::Data(string xmlstring) throw(WorkflowFormatException)
    {
    	DOMDocument* doc = XMLUtils::Instance()->deserialize(xmlstring);
    	DOMElement* element = doc->getDocumentElement();
    	char* name = XMLString::transcode(element->getTagName()); 
    	if (strcmp(name,"data")) {
    		ostringstream message; 
			message << "Error: Data element must have name \"data\", but has name \""<< name << "\"";
    		throw WorkflowFormatException(message.str());	
    	} 
        data = element;
        dataText = NULL;
        setType();
    }
    
	Data::~Data()
	{
	  if (data != NULL) data->release();
	  if (dataText != NULL) delete dataText;
	}
	
	void Data::setType() {
		if (data==NULL) _type = TYPE_EMPTY;
		_type = TYPE_DATA;
		DOMNodeList* le = data->getChildNodes();
		for (XMLSize_t i = 0; i<le->getLength(); i++) {
			DOMNode* node = le->item(i);
			const XMLCh* name = node->getNodeName(); 
			if (XMLString::equals(name,X("file"))) _type = TYPE_FILE;
			else if (XMLString::equals(name,X("volume"))) _type = TYPE_VOLUME;
			else if (XMLString::equals(name,X("parameter"))) _type = TYPE_PARAMETER;
			else if (XMLString::equals(name,X("soapenv:Fault"))) _type = TYPE_FAULT;
		}
	}
	
	DOMElement* Data::toElement(DOMDocument* doc)
	{
		if (data==NULL) return NULL;
		if (doc!=NULL) {
			DOMDocument * owner = data->getOwnerDocument();
			if (owner != doc) {
//				cout << "wrong owner, importing node..." << endl;
				DOMNode* newdata = doc->importNode(data,true); 
				return (DOMElement*) newdata;
			} 
//			else {
//				cout << "right owner, nothing to import..." << endl;	
//			}
		}
		return data;
	}
	
	string* Data::getText()
	{
		if (dataText == NULL) {
			dataText = new string(S(data->getTextContent()));
			dataText = &trim(*dataText);
		}
		return dataText;
	}
	
	string* Data::toString()
	{
		return XMLUtils::Instance()->serialize(data,false);
	}
	
	string& Data::trim(string& s)
	{
	 string& r=s.erase(s.find_last_not_of(" ")+1);
	 return r.erase(0,r.find_first_not_of(" "));
	}
	
	Data* Data::deepCopy()
	{
		DOMDocument* doc = XMLUtils::Instance()->createEmptyDocument(false);
		// toElement imports the DOM node which makes the deep copy.
		DOMElement* element = toElement(doc);
		return new Data(element);
	}

}

ostream& operator<<(ostream &out, gwdl::Data &data) 
{	
	DOMNode* node = data.toElement(XMLUtils::Instance()->createEmptyDocument(false));
	XMLUtils::Instance()->serialize(out,node,true);
	return out;
}

