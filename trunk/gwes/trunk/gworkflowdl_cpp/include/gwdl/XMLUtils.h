/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef XMLUTILS_H_
#define XMLUTILS_H_
// gwdl
#include <gwdl/WorkflowFormatException.h>
#include <gwdl/XMLDOMErrorHandler.h>
//fhglog
#include <fhglog/fhglog.hpp>
// libxml2
#include <libxml/xpathInternals.h>
//xerces
#include <xercesc/dom/DOM.hpp>
// std
#include <ostream>
#include <string>

namespace gwdl
{

/**
 * This class contains helper utilities for serializing and deserializing XML.
 * 
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */ 
class XMLUtils
{
private:
	static XMLUtils* _instance;
	XMLDOMErrorHandler* _errorHandler;
	fhg::log::logger_t _logger;
	int initializeXerces();
	void terminateXerces();
	int initializeLibxml2();
	void terminateLibxml2();

protected:
	XMLUtils();

public:

	/**
	 * Get a the singleton instance of this XMLUtils.
	 * This automatically initiates the XML parser.
	 * @return The singleton.
	 */ 
	static XMLUtils* Instance();

	/**
	 * Destructor for XMLUtils.
	 * This terminates the XML parser.
	 */	
	virtual ~XMLUtils();
	
	/**
	 * Serialize the DOMNode to an ostream.
	 * @param os The output stream.
	 * @param node The DOM node to serialize.
	 * @param pretty Format pretty print
	 * @return The output stream. 
	 */
	std::ostream& serialize (std::ostream& os, const XERCES_CPP_NAMESPACE::DOMNode* node, bool pretty);
	
	/**
	 * Xerces: Serialize the DOMDocument to an ostream.
	 * @param os The output stream.
	 * @param doc The DOM document to serialize.
	 * @param pretty Format pretty print
	 * @return The output stream. 
	 */
	std::ostream& serialize (std::ostream& os, const XERCES_CPP_NAMESPACE::DOMDocument* doc, bool pretty);

	/**
	 * Xerces: Serialize the DOMNode to a string.
	 * @param node The DOM node to serialize.
	 * @param pretty Format pretty print
	 * @return The xml string. 
	 */
	std::string* serialize (const XERCES_CPP_NAMESPACE::DOMNode* node, bool pretty);

	/**
	 * Xerces: Serialize the DOMDocument to a string.
	 * @param node The DOM document to serialize.
	 * @param pretty Format pretty print
	 * @return The xml string. 
	 */
	std::string* serialize (const XERCES_CPP_NAMESPACE::DOMDocument* doc, bool pretty); 
	
	/**
	 * Xerces: Deserialize the DOMNode from an XML string.
	 * @param xmlstring The XML string containing a node.
	 * @return The corresponding DOMNode.
	 */
	XERCES_CPP_NAMESPACE::DOMDocument* deserialize (const std::string& xmlstring, bool validating = false) throw (WorkflowFormatException);

	/**
	 * Libxml2: Serialize the DOMDocument to a string.
	 * @param doc The DOM document to serialize.
	 * @param pretty Format pretty print
	 * @return The xml string. 
	 */
	std::string serializeLibxml2 (const xmlDocPtr doc, bool pretty); 
	
	/**
	 * Libxml2: Serialize DOM node to a string.
	 * @param node The DOM node to serialize.
	 * @param pretty Format pretty print
	 * @return The xml string. 
	 */
	std::string serializeLibxml2 (const xmlNodePtr node, bool pretty); 

	/**
	 * Libxml2: Deserialize the xmlDocPtr from an XML string.
	 * @param xmlstring The XML string containing a node.
	 * @return The corresponding xmlDocPtr.
	 */
	xmlDocPtr deserializeLibxml2 (const std::string& xmlstring, bool validating = false) throw (WorkflowFormatException);
	
	/**
	 * Libxml2: Deserialize the xmlDocPtr from an XML file.
	 * @param filename The filename containing the XML document.
	 * @return The corresponding xmlDocPtr.
	 */
	xmlDocPtr deserializeFileLibxml2(const std::string& filename, bool validating = false) throw (WorkflowFormatException);
	
	/**
	 * Creates an empty document.
	 * @param gwdlnamespace Set to <code>true</code> if you want to use the GWorkflowDL namespace.
	 * @return The document.
	 */	
	XERCES_CPP_NAMESPACE::DOMDocument* createEmptyDocument(bool gwdlnamespace);

	/**
	 * Trim leading and trailing spaces, tabs etc. from string.
	 */
	static void trim(std::string& s);
	
	/**
	 * Get the text contents of the all the xml elements
	 * that are siblings or children of a given xml node.
	 */
	std::string getText(const std::string& xml);
	
	/**
	 * Get the text contents of the all the xml elements
	 * that are siblings or children of a given xml node.
	 */
	void getText(std::ostringstream &out, xmlNodePtr nodeP);

	static bool endsWith(const std::string& s1, const std::string& s2);

	static bool startsWith(const std::string& s1, const std::string& s2);
	
};

}

#endif /*XMLUTILS_H_*/
