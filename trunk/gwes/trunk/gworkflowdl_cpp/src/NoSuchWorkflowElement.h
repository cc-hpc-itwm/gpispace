/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef NOSUCHWORKFLOWELEMENT_H_
#define NOSUCHWORKFLOWELEMENT_H_
#include <string>

using namespace std;

namespace gwdl
{

/**
 * This Exception is thrown when a workflow element is requested, which is not available within the workflow.
 * 
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */ 
class NoSuchWorkflowElement
{
public:
		string message;
		NoSuchWorkflowElement(string _message) 
		{
			message = _message;					
		}
};

}

#endif /*NOSUCHWORKFLOWELEMENT_H_*/
