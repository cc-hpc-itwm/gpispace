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

struct mutex_lock {
	mutex_lock(pthread_mutex_t &mutex) : mtx(mutex) {
		pthread_mutex_lock(&mtx);
	}
	~mutex_lock() {
		pthread_mutex_unlock(&mtx);
	}

	pthread_mutex_t &mtx;
};

XMLUtils* XMLUtils::_instance=0;

XMLUtils* XMLUtils::Instance() {
	if (_instance==0) {
		_instance = new XMLUtils;
	}
	return _instance;
}

XMLUtils::XMLUtils() : _logger(fhg::log::getLogger("gwdl"))
{
	pthread_mutex_init(&_lock, NULL);
	LOG_DEBUG(_logger, "XMLUtils() ...");
	// initialize libxml2 parser. This should only be called ONCE!
	xmlInitParser();
	LIBXML_TEST_VERSION
}

XMLUtils::~XMLUtils()
{
	LOG_DEBUG(_logger, "~XMLUtils() ...");
	// cleanup xml parser
	xmlCleanupParser();
	pthread_mutex_destroy(&_lock);
}

string XMLUtils::serializeLibxml2Doc(const xmlDocPtr doc, bool pretty) {
	// Alternative without setting XML_SAVE_NO_DECL: 
	//    xmlChar *xmlbuff;
	//    int buffersize;
	//    xmlDocDumpFormatMemory(doc, &xmlbuff, &buffersize, pretty);
	int option = XML_SAVE_NO_DECL;
	if (pretty) option += XML_SAVE_FORMAT;
	mutex_lock lock(_lock);
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
	LOG_DEBUG(_logger, "deserializeLibxml2(" << xmlstring << ")...");
	if (validating) LOG_WARN(_logger, "XMLUtils::deserialize(): Validation not yet supported for libxml2");
	xmlChar* duped(xmlCharStrdup(xmlstring.c_str()));
	mutex_lock lock(_lock);
	xmlDocPtr res(xmlParseDoc(duped));
	xmlFree(duped);
	if (!res) {
		xmlErrorPtr error = xmlGetLastError();
		ostringstream message;
		if (error) {
			message << "XML ERROR (line:" << error->line << "/column:" << error->int2 << "): " << error->message;
			xmlResetError(error);
		} else {
			message << "XML ERROR: Unkown error ";
		}
		LOG_ERROR(_logger, message.str() << "when parsing:\n" << xmlstring);
		throw WorkflowFormatException(message.str());
	} 
	return res;
}

xmlDocPtr XMLUtils::deserializeFileLibxml2(const std::string& filename, bool validating) throw (WorkflowFormatException) {
	if (validating) LOG_WARN(_logger, "XMLUtils::deserialize(): Validation not yet supported for libxml2");
	LOG_DEBUG(_logger, "xmlParseFile("<< filename.c_str() <<") ...");
	mutex_lock lock(_lock);
	xmlDocPtr docP = xmlParseFile(filename.c_str());
	if (!docP) {
		xmlErrorPtr error = xmlGetLastError();
		ostringstream message;
		if (error) {
			message << "XML ERROR when parsing file '" << filename << "' (line:" << error->line << "/column:" << error->int2 << "): " << error->message;
			xmlResetError(error);
		} else {
			message << "XML ERROR: Unkown error when parsing file '" << filename << "'";
		}
		LOG_ERROR(_logger, message.str());
		throw WorkflowFormatException(message.str());
	} 
	return docP;
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
	LOG_DEBUG(logger_t(getLogger("gwdl")), "reading file '"<< filename << "'...");
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
