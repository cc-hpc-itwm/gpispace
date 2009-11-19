/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef XMLDOMERRORHANDLER_H_
#define XMLDOMERRORHANDLER_H_
//xerces
#include <xercesc/dom/DOM.hpp>
//std
#include <string>

namespace gwdl
{

/**
 * This class handles the exceptions reported by the DOM XML parser.
 * 
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */ 
class XMLDOMErrorHandler : public XERCES_CPP_NAMESPACE::DOMErrorHandler
{
public:
	XMLDOMErrorHandler();
	virtual ~XMLDOMErrorHandler();
	virtual bool handleError(const XERCES_CPP_NAMESPACE::DOMError &domError);
	bool hasError;
	short error;
	std::string message;
	void reset();
};

}

#endif /*XMLDOMERRORHANDLER_H_*/
