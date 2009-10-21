/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef ACTIVITYEXCEPTION_H_
#define ACTIVITYEXCEPTION_H_
// std
#include <stdexcept>
#include <string>

namespace gwes
{

/**
 * An ActivityException is thrown if an error during the activity execution occurs.
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class ActivityException : public std::runtime_error {
	
	public:
		std::string message;
		ActivityException(const std::string& _message) 
          : std::runtime_error(message)
		{
			message = _message;					
		}

        virtual ~ActivityException() throw() {}
}; // end class ActivityException

} // end namespace gwes

#endif /*ACTIVITYEXCEPTION_H_*/
