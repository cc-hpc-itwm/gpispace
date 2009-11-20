/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// gwdl
#include <gwdl/XMLUtils.h>
#include <gwdl/Defines.h>
#include <gwdl/Libxml2Builder.h>
// libxml2
#include <libxml/xmlsave.h>
#include <tr1/memory>

using namespace std;

namespace gwdl
{

XMLUtils* XMLUtils::_instance=0;

XMLUtils* XMLUtils::Instance() {
	if (_instance==0) {
		_instance = new XMLUtils;
	}
	return _instance;
}

XMLUtils::XMLUtils() : _logger(fhg::log::getLogger("gwdl"))
{
	LOG_DEBUG(_logger, "XMLUtils() ...");
	initializeLibxml2();
}

XMLUtils::~XMLUtils()
{
	LOG_DEBUG(_logger, "~XMLUtils() ...");
	terminateLibxml2();
	delete _instance;
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

string XMLUtils::serializeLibxml2Doc(const xmlDocPtr doc, bool pretty) {
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

/**
 * Serializes a single XML node to string.
 * If node points to a list of several nodes, only the first node is serialized.
 */ 
string XMLUtils::serializeLibxml2Node(const xmlNodePtr node, bool pretty) {
	string ret;
	xmlNodePtr curNodeP = node;
	xmlNodePtr curNodeCopyP;
	xmlDocPtr docP;
	if(curNodeP) {
		docP = xmlNewDoc((const xmlChar*)"1.0");
		curNodeCopyP = xmlDocCopyNode(curNodeP,docP,1);
		xmlDocSetRootElement(docP,curNodeCopyP);
		ret += serializeLibxml2Doc(docP,pretty);
		xmlFreeDoc(docP);
		curNodeP = curNodeP->next;
	}
	return ret;
}

/**
 * Serializes XML node or list of nodes to string.
 * If node points to a list of several nodes, each node is serialized
 * separately as one document and concatenated to one string.
 */ 
string XMLUtils::serializeLibxml2NodeList(const xmlNodePtr node, bool pretty) {
	string ret;
	xmlNodePtr curNodeP = node;
	xmlNodePtr curNodeCopyP;
	xmlDocPtr docP;
	while(curNodeP) {
		docP = xmlNewDoc((const xmlChar*)"1.0");
		curNodeCopyP = xmlDocCopyNode(curNodeP,docP,1);
		xmlDocSetRootElement(docP,curNodeCopyP);
		ret += serializeLibxml2Doc(docP,pretty);
		xmlFreeDoc(docP);
		curNodeP = curNodeP->next;
	}
	return ret;
}

xmlDocPtr XMLUtils::deserializeLibxml2(const std::string& xmlstring, bool validating) throw (WorkflowFormatException) {
	if (validating) LOG_WARN(_logger, "XMLUtils::deserialize(): Validation not yet supported for libxml2");
	xmlChar* duped(xmlCharStrdup(xmlstring.c_str()));
	xmlDocPtr res(xmlParseDoc(duped));
	xmlFree(duped);
	return res;
}

xmlDocPtr XMLUtils::deserializeFileLibxml2(const std::string& filename, bool validating) throw (WorkflowFormatException) {
	if (validating) LOG_WARN(_logger, "XMLUtils::deserialize(): Validation not yet supported for libxml2");
	return xmlParseFile(filename.c_str());
}

void XMLUtils::trim(string& s) {
	s.erase(s.find_last_not_of(" \t\f\v\n\r")+1).erase(0,s.find_first_not_of(" \t\f\v\n\r"));
}

/**
 * Get the text contents of the all the xml elements
 * that are siblings or children of a given xml node.
 */
string XMLUtils::getText(const string& xml) {
	xmlDocPtr docP = deserializeLibxml2(xml,false);
	xmlNodePtr rootElementP = xmlDocGetRootElement(docP);
	ostringstream out;
	getText(out,rootElementP);
	xmlFreeDoc(docP);
	return out.str();
}

/**
 * Get the text contents of the all the xml elements
 * that are siblings or children of a given xml node.
 */
void XMLUtils::getText(ostringstream &out, xmlNodePtr nodeP)
{
	xmlNodePtr cur_node = NULL;

	for (cur_node = nodeP; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_TEXT_NODE) {
			out << cur_node->content;
		}
		getText(out, cur_node->children);
	}
}

bool XMLUtils::endsWith(const string& str, const string& substr) {
	if ( substr.size() > str.size() ) return false;
	if ( str.compare(str.size()-substr.size(),substr.size(),substr ) == 0) {
		return true;
	}
	return false;
}

bool XMLUtils::startsWith(const string& str, const string& substr) {
	if ( substr.size() > str.size() ) return false;
	if ( str.compare(0,substr.size(),substr ) == 0) {
		return true;
	}
	return false;
}

string XMLUtils::readFile(const string& filename) throw (WorkflowFormatException) {
	// read file
	ifstream file(filename.c_str());
	ostringstream workflowS;
	if (file.is_open()) {
		char c;
		while (file.get(c)) {
			workflowS << c;
		}
		file.close();
	} else {
		ostringstream message; 
		message << "Unable to open file " << filename << ": " << strerror(errno);
		LOG_ERROR(logger_t(getLogger("gwdl")), message);
		throw WorkflowFormatException(message.str());
	}
	return workflowS.str();
}

} // end namespace gwdl
