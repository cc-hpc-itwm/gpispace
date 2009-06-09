#ifndef XMLDOMERRORHANDLER_H_
#define XMLDOMERRORHANDLER_H_
//xerces
#include <xercesc/dom/DOM.hpp>
XERCES_CPP_NAMESPACE_USE
//std
#include <string>

using namespace std;

namespace gwdl
{

/**
 * This class handles the exceptions reported by the DOM XML parser.
 * 
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */ 
class XMLDOMErrorHandler : public DOMErrorHandler
{
public:
	XMLDOMErrorHandler();
	virtual ~XMLDOMErrorHandler();
	virtual bool handleError(const DOMError &domError);
	bool hasError;
	short error;
	string message;
	void reset();
};

}

#endif /*XMLDOMERRORHANDLER_H_*/
