/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef WORKFLOWFORMATEXCEPTION_H_
#define WORKFLOWFORMATEXCEPTION_H_

//std
#include <stdexcept>
#include <string>

namespace gwdl
{

/**
 * This Exception is thrown when the workflow document does not match the specification.
 * 
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */ 
class WorkflowFormatException : public std::runtime_error
{
public:
		WorkflowFormatException(const std::string& a_message)
          : std::runtime_error(a_message)
		{
		}

        ~WorkflowFormatException() throw() {}
};

}

#endif /*WORKFLOWFORMATEXCEPTION_H_*/
