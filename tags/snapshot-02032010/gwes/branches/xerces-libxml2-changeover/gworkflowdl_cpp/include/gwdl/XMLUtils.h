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
//fhglog
#include <fhglog/fhglog.hpp>
// libxml2
#include <libxml/xpathInternals.h>
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
	fhg::log::logger_t _logger;
	pthread_mutex_t _lock;

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
	 * Libxml2: Serialize the DOMDocument to a string.
	 * @param doc The DOM document to serialize.
	 * @param pretty Format pretty print
	 * @return The xml string. 
	 */
	std::string serializeLibxml2Doc (const xmlDocPtr doc, bool pretty); 
	
	/**
	 * Serializes a single XML node to string.
	 * If node points to a list of several nodes, only the first node is serialized.
	 */ 
	std::string serializeLibxml2Node(const xmlNodePtr node, bool pretty);

    /**
	 * Serializes XML node or list of nodes to string.
	 * If node points to a list of several nodes, each node is serialized
	 * separately as one document and concatenated to one string.
	 */ 
	std::string serializeLibxml2NodeList(const xmlNodePtr node, bool pretty);

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

	static bool endsWith(const std::string& str, const std::string& substr);

	static bool startsWith(const std::string& str, const std::string& substr);
	
	static std::string readFile(const std::string& filename) throw (WorkflowFormatException);
	
};

}

#endif /*XMLUTILS_H_*/
