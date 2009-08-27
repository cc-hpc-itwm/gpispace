/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// gwdl
#include <gwdl/XMLUtils.h>
#include <gwdl/Defines.h>
// libxml2
#include <libxml/xmlsave.h>
// xerces-c
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
// std
#include <iostream>

XERCES_CPP_NAMESPACE_USE
using namespace std;

#define X(str) XMLString::transcode((const char*)& str)

namespace gwdl
{

XMLUtils* XMLUtils::_instance=0;

XMLUtils* XMLUtils::Instance() {
	if (_instance==0) {
		_instance = new XMLUtils;
	}
	return _instance;
}

XMLUtils::XMLUtils()
{
	cout << "XMLUtils::XMLUtils() ..." << endl;
	initializeXerces();
	initializeLibxml2();
	_errorHandler = new XMLDOMErrorHandler();
}

XMLUtils::~XMLUtils()
{
	cout << "XMLUtils::~XMLUtils() ..." << endl;
	 terminateXerces();
	 terminateLibxml2();
	 delete _instance;
	 delete _errorHandler;
}

int XMLUtils::initializeXerces()
{
	try {
      XMLPlatformUtils::Initialize();
    }
    catch (const XMLException& toCatch) {
      return 1;
    }
    return 0;
}

void XMLUtils::terminateXerces()
{
	 XMLPlatformUtils::Terminate();
}

int XMLUtils::initializeLibxml2()
{
	// init libxml2 parser
	xmlInitParser();
	LIBXML_TEST_VERSION
    return 0;
}

void XMLUtils::terminateLibxml2()
{
	// cleanup xml parser
	xmlCleanupParser();
}

ostream& XMLUtils::serialize(ostream& os, const DOMNode* node, bool pretty)
{
    DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(X("LS"));
    DOMWriter* writer = ((DOMImplementationLS*)impl)->createDOMWriter();
    // set features
    if (pretty && writer->canSetFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true)) writer->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true); 
    // error handler
    writer->setErrorHandler(_errorHandler);
    // write
	XMLCh* xmlstr = writer->writeToString(*node);
	os << XMLString::transcode((XMLCh*)xmlstr);
	// check for errors
	if (_errorHandler->hasError) 
	{
		_errorHandler->reset();
		cerr << _errorHandler->message << endl;;
	}
	return os;
}

ostream& XMLUtils::serialize(ostream& os, const DOMDocument* doc, bool pretty)
{
    DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(X("LS"));
    DOMWriter* writer = ((DOMImplementationLS*)impl)->createDOMWriter();
    // set features
    if (pretty && writer->canSetFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true)) writer->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true); 
    // error handler
    writer->setErrorHandler(_errorHandler);
    // write
	XMLCh* xmlstr = writer->writeToString(*doc);
	os << XMLString::transcode((XMLCh*)xmlstr);
	// check for errors
	if (_errorHandler->hasError) 
	{
		_errorHandler->reset();
		cerr << _errorHandler->message << endl;;
	}
	return os;
}

string* XMLUtils::serialize (const DOMNode* node, bool pretty) 
{
    DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(X("LS"));
    DOMWriter* writer = ((DOMImplementationLS*)impl)->createDOMWriter();
    // set features
    if (pretty && writer->canSetFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true)) writer->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true);
    // error handler
    writer->setErrorHandler(_errorHandler);
    // write 
	XMLCh* xmlstr = writer->writeToString(*node);
	string *str = new string(XMLString::transcode((XMLCh*)xmlstr));
	// check for errors
	if (_errorHandler->hasError) 
	{
		_errorHandler->reset();
		cerr << _errorHandler->message << endl;;
	}
	
	return str;
}

string* XMLUtils::serialize (const DOMDocument* doc, bool pretty) 
{
    DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(X("LS"));
    DOMWriter* writer = ((DOMImplementationLS*)impl)->createDOMWriter();
    // set features
    if (pretty && writer->canSetFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true)) writer->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true);
    // error handler
    writer->setErrorHandler(_errorHandler);
    // write 
	XMLCh* xmlstr = writer->writeToString(*doc);
	string *str = new string(XMLString::transcode((XMLCh*)xmlstr));
	// check for errors
	if (_errorHandler->hasError) 
	{
		_errorHandler->reset();
		cerr << _errorHandler->message << endl;;
	}
	
	return str;
}

DOMDocument* XMLUtils::deserialize (const string& xmlstring, bool validating) throw (WorkflowFormatException)
{
    DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(X("LS"));
    DOMBuilder* parser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
    DOMDocument *doc;
    
    // set parser features
	if (validating && parser->canSetFeature(XMLUni::fgDOMValidation, true)) parser->setFeature(XMLUni::fgDOMValidation, true);
    if (parser->canSetFeature(XMLUni::fgDOMNamespaces, true)) parser->setFeature(XMLUni::fgDOMNamespaces, true);
    if (parser->canSetFeature(XMLUni::fgDOMDatatypeNormalization, true)) parser->setFeature(XMLUni::fgDOMDatatypeNormalization, true);
    
    //parser->setFeature(XMLUni::fgXercesSchema, false);
    //parser->setFeature(XMLUni::fgXercesSchemaFullChecking, false);
    
    // error handler
    parser->setErrorHandler(_errorHandler);
        
    // input source
    const char * xmlbyte = xmlstring.c_str();
	MemBufInputSource* memBufIS = new MemBufInputSource( (const XMLByte*)xmlbyte, strlen(xmlbyte), "id", false);
    Wrapper4InputSource *wrapper=new Wrapper4InputSource(memBufIS,false);
    
    try 
    {
		doc = parser->parse(*wrapper);
    } 
    catch (const XMLException& toCatch) 
    {
        char* message = XMLString::transcode(toCatch.getMessage());
        cerr << "Exception message is: \n" << message << "\n";
        XMLString::release(&message);
        throw WorkflowFormatException(message);
    }
    catch (const DOMException& toCatch) 
    {
        char* message = XMLString::transcode(toCatch.msg);
        cerr << "Exception message is: \n" << message << "\n";
        XMLString::release(&message);
        throw WorkflowFormatException(message);
    }
    catch (...) {
        cerr << "Unexpected Exception \n" ;
        throw WorkflowFormatException("Unexpected Exception");
    }
    
    if (_errorHandler->hasError) 
	{
		_errorHandler->reset();
		//throw WorkflowFormatException(_errorHandler->message);
		cout << _errorHandler->message << endl; 
	}
    	
	return doc;
}

string XMLUtils::serializeLibxml2(const xmlDocPtr doc, bool pretty) {
	// Alternative without setting XML_SAVE_NO_DECL: 
	//    xmlChar *xmlbuff;
	//    int buffersize;
	//    xmlDocDumpFormatMemory(doc, &xmlbuff, &buffersize, pretty);
	int option = XML_SAVE_NO_DECL;
	if (pretty) option += XML_SAVE_FORMAT;
	xmlBufferPtr buffer = xmlBufferCreate();
    xmlSaveCtxtPtr ctxt = xmlSaveToBuffer(buffer, "UTF-8", option);
    xmlSaveDoc(ctxt, doc);
    xmlSaveClose(ctxt);
    string str = string((const char*)buffer->content);
    xmlBufferFree(buffer);
    return str;
}

string XMLUtils::serializeLibxml2(const xmlNodePtr node, bool pretty) {
	cout << "XMLUtils::serializeLibxml2() ..." << endl;
	xmlDocPtr docP = xmlNewDoc((const xmlChar*)"1.0");
	xmlNodePtr nodecopy = xmlDocCopyNode(node,docP,1);
	xmlDocSetRootElement(docP,nodecopy);
	string ret = serializeLibxml2(docP,pretty);
	xmlFreeDoc(docP);
	return ret;
}

xmlDocPtr XMLUtils::deserializeLibxml2(const std::string& xmlstring, bool validating) throw (WorkflowFormatException) {
	if (validating) cerr << "XMLUtils::deserialize(): Validation not yet supported for libxml2" << endl;
	return xmlParseDoc(xmlCharStrdup(xmlstring.c_str()));
}

DOMDocument* XMLUtils::createEmptyDocument(bool gwdlnamespace)
{
	XMLCh* ns = 0;
	if (gwdlnamespace) ns = X(SCHEMA_wfSpace);
	DOMImplementation* impl (DOMImplementationRegistry::getDOMImplementation (X("LS")));
	if (impl==NULL)
	{
	    XERCES_STD_QUALIFIER cerr << "Requested implementation is not supported" << XERCES_STD_QUALIFIER endl;
    }
	
	DOMDocument* doc = impl->createDocument(ns,X("workflow"),0);
	return doc;
}


}
