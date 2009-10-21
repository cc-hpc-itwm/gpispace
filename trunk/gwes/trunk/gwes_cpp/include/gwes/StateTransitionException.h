/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef STATETRANSITIONEXCEPTION_H_
#define STATETRANSITIONEXCEPTION_H_
// std
#include <stdexcept>
#include <string>

namespace gwes
{

/**
 * A StateTransitionException is thrown if workflow or activity is in the wrong state for
 * invoking a specific action. For example, a workflow must be in state "SUSPENDED" in order
 * to be resumed. 
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class StateTransitionException : public std::runtime_error {
	
	public:
		StateTransitionException(const std::string& a_message) 
          : std::runtime_error(a_message)
		{
		}

        ~StateTransitionException() throw() {}
}; // end class StateTransitionException

} // end namespace gwes

#endif /*STATETRANSITIONEXCEPTION_H_*/
