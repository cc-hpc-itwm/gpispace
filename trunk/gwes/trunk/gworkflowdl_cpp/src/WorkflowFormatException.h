/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef WORKFLOWFORMATEXCEPTION_H_
#define WORKFLOWFORMATEXCEPTION_H_
#include <string>

using namespace std;

namespace gwdl
{

/**
 * This Exception is thrown when the workflow document does not match the specification.
 * 
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */ 
class WorkflowFormatException
{
public:
		string message;
		WorkflowFormatException(string _message) 
		{
			message = _message;					
		}
};

}

#endif /*WORKFLOWFORMATEXCEPTION_H_*/
