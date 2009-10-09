/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// gwdl
#include <gwdl/XMLUtils.h>
#include <gwdl/Data.h>

#include <gwdl/XMLTranscode.hpp>

using namespace std;
using namespace gwdl;
XERCES_CPP_NAMESPACE_USE

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

Data::Data(const string& xmlstring) throw(WorkflowFormatException)
{
	DOMDocument* doc = XMLUtils::Instance()->deserialize(xmlstring);
    assert(doc);
	DOMElement* element = doc->getDocumentElement();
    assert(element);
	char* name = XMLString::transcode(element->getTagName()); 
	if (strcmp(name,"data")) {
		ostringstream message; 
		message << "Error: Data element must have name \"data\", but has name \""<< name << "\"";
                XMLString::release(&name);
		throw WorkflowFormatException(message.str());	
	} 
                XMLString::release(&name);
	data = element;
    // FIXME: replace this with a member variable std::string, not a pointer!
	dataText = NULL;
	setType();
}

Data::~Data()
{
// FIXME: if Data(DOMElement) is used, who owns the document?
//        if Data(string) is used, who releases the document?
//        what happens if Data(DOMElement) is used several times?
//    if (data != NULL) { data->getOwnerDocument()->release(); data = NULL; }
	if (dataText != NULL) { delete dataText; dataText = NULL; }
}

void Data::setType() {
	if (data==NULL) _type = TYPE_EMPTY;
	_type = TYPE_DATA;
	DOMNodeList* le = data->getChildNodes();
	for (XMLSize_t i = 0; i<le->getLength(); i++) {
		DOMNode* node = le->item(i);
		const XMLCh* name = node->getNodeName(); 
		if (XMLString::equals(name,S("file"))) _type = TYPE_FILE;
		else if (XMLString::equals(name,S("volume"))) _type = TYPE_VOLUME;
		else if (XMLString::equals(name,S("parameter"))) _type = TYPE_PARAMETER;
		else if (XMLString::equals(name,S("soapenv:Fault"))) _type = TYPE_FAULT;
	}
}

DOMElement* Data::toElement(DOMDocument* doc)
{
	if (data==NULL) return NULL;
	if (doc!=NULL) {
		DOMDocument * owner = data->getOwnerDocument();
		if (owner != doc) {
			DOMNode* newdata = doc->importNode(data,true); 
			return (DOMElement*) newdata;
		} 
	}
	return data;
}

string* Data::getText()
{
	if (dataText == NULL) {
          char * cntx(S(data->getTextContent()));
		dataText = new string(cntx);
                XMLString::release(&cntx);
		trim(*dataText);
	}
	return dataText;
}

string* Data::toString() const
{
	return XMLUtils::Instance()->serialize(data,false);
}

void Data::trim(string& s)
{
	s.erase(s.find_last_not_of(" \t\f\v\n\r")+1).erase(0,s.find_first_not_of(" \t\f\v\n\r"));
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

