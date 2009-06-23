/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef COMMANDLINEACTIVITY_H_
#define COMMANDLINEACTIVITY_H_
// std
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <string>
// gwdl
#include <gwdl/OperationCandidate.h>
// gwes
#include <gwes/Activity.h>

#define GWES_TEMP_DIRECTORY "/tmp/.gwes"

namespace gwes
{

/**
 * The CommandLineActivity is a specific implementation of Activity which invokes local command line programs regarding
 * a certain command line syntax.
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */ 
class CommandLineActivity : public Activity
{
	
private:
	std::string _workingDirectory;
	std::string _stdoutfn;
	std::string _stderrfn;
	std::string _exitcodefn;
	std::string generateOutputDataURL(std::string edgeExpression);
	std::string convertUrlToLocalPath(std::string url);
	std::string execute(std::string commandline);

public:
	CommandLineActivity(WorkflowHandler* handler, gwdl::OperationCandidate* operation);
	virtual ~CommandLineActivity();

	/**
	 * Initiate this activity. Status should switch to INITIATED. Method should only work if the status was
	 * UNDEFINED before. 
	 */
	virtual void initiateActivity() throw (ActivityException,StateTransitionException);

	/**
	 * Start this activity. Status should switch to RUNNING. 
	 */
	virtual void startActivity() throw (ActivityException,StateTransitionException,gwdl::WorkflowFormatException);

	/**
	 * Suspend this activity. Status should switch to SUSPENDED. 
	 */
	virtual void suspendActivity() throw (ActivityException,StateTransitionException);

	/**
	 * Resume this activity. Status should switch to RUNNING. 
	 */
	virtual void resumeActivity() throw (ActivityException,StateTransitionException);

	/**
	 * Abort this activity. Status should switch to TERMINATED.
	 */
	virtual void abortActivity() throw (ActivityException,StateTransitionException);

	/**
	 * Restart this activity. Status should switch to INITIATED. 
	 */
	virtual void restartActivity() throw (ActivityException,StateTransitionException);
	
};

}

#endif /*COMMANDLINEACTIVITY_H_*/
