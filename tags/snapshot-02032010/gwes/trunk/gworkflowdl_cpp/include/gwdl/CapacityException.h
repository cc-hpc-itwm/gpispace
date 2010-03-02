/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef CAPACITYEXCEPTION_H_
#define CAPACITYEXCEPTION_H_
#include <stdexcept>
#include <string>

namespace gwdl
{

/**
 * A CapacityException is thrown when the number of tokens on a place exceeds its capacity.
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class CapacityException : public std::runtime_error {
	
	public:
		CapacityException(const std::string& a_message)
          : std::runtime_error(a_message)

		{
		}

        ~CapacityException() throw() { }

}; // end class CapacityException

} // end namespace gwdl

#endif /*CAPACITYEXCEPTION_H_*/
