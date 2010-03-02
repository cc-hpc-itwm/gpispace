/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef XPATHEXCEPTION_H_
#define XPATHEXCEPTION_H_
// std
#include <stdexcept>
#include <string>

namespace gwes
{

/**
 * An XPathException is thrown if an error during XPath evaluation occurs.
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class XPathException : public std::runtime_error {
	
	public:
		XPathException(const std::string& a_message) 
          : std::runtime_error(a_message)
		{
		}

        ~XPathException() throw() {}
}; // end class XPathException

} // end namespace gwes

#endif /*XPATHEXCEPTION_H_*/
