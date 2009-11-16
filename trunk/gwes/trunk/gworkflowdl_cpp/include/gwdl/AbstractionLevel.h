/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef ABSTRACTIONLEVEL_H_
#define ABSTRACTIONLEVEL_H_

namespace gwdl {

/**
 * AbstractionLevel of Operations.
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class AbstractionLevel {

public:
	
    enum abstraction_t
    {
    /**
     * no operation
     */
     BLACK = -1,
    /**
     * no Classoperation specified
     */
     RED = 0,
    /**
     * WSOperationClass owl specified
     */
     YELLOW = 1,

    /**
     * List of WSOperations given, non (or more than one) selected
     */
     BLUE   = 2,

    /**
     * List given and one WSOperation selected
     */
     GREEN  = 3
    };

    AbstractionLevel() {}
	virtual ~AbstractionLevel();

}; // end class AbstractionLevel 

} // end namespace gwdl


#endif /*ABSTRACTIONLEVEL_H_*/
