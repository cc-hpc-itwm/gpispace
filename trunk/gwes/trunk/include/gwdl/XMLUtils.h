#ifndef XMLUTILS_H_
#define XMLUTILS_H_
//xerces
#include <xercesc/dom/DOM.hpp>
// std
#include <ostream>
#include <string>
// gwdl
#include <gwdl/WorkflowFormatException.h>
#include <gwdl/XMLDOMErrorHandler.h>

using namespace std;
XERCES_CPP_NAMESPACE_USE

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
	int initializeXerces();
	void terminateXerces();

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
	ostream& serialize (ostream& os, const DOMNode* node, bool pretty);
	
	/**
	 * Serialize the DOMDocument to an ostream.
	 * @param os The output stream.
	 * @param doc The DOM document to serialize.
	 * @param pretty Format pretty print
	 * @return The output stream. 
	 */
	ostream& serialize (ostream& os, const DOMDocument* doc, bool pretty);

	/**
	 * Serialize the DOMNode to a string.
	 * @param node The DOM node to serialize.
	 * @param pretty Format pretty print
	 * @return The xml string. 
	 */
	string* serialize (const DOMNode* node, bool pretty);

	/**
	 * Serialize the DOMDocument to a string.
	 * @param node The DOM document to serialize.
	 * @param pretty Format pretty print
	 * @return The xml string. 
	 */
	string* serialize (const DOMDocument* doc, bool pretty); 
	
	/**
	 * Deserialize the DOMNode from a XML string.
	 * @param xmlstring The XML string containing a node.
	 * @return The corresponding DOMNode.
	 */
	DOMDocument* deserialize (string& xmlstring, bool validating = false) throw (WorkflowFormatException);

	/**
	 * Creates an empty document.
	 * @param gwdlnamespace Set to <code>true</code> if you want to use the GWorkflowDL namespace.
	 * @return The document.
	 */	
	DOMDocument* createEmptyDocument(bool gwdlnamespace);

};

}

#endif /*XMLUTILS_H_*/
