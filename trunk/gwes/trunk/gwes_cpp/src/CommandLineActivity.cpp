/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//gwes
#include <gwes/CommandLineActivity.h>
#include <gwes/TransitionOccurrence.h>
#include <gwes/Utils.h>
//gwdl
#include <gwdl/Token.h>
//std
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <map>
#include <string>

using namespace std;

namespace gwes
{

CommandLineActivity::CommandLineActivity(WorkflowHandler* handler, TransitionOccurrence* toP, gwdl::OperationCandidate* operationP) : Activity(handler, toP, "CommandLineActivity", operationP)
{
}

CommandLineActivity::~CommandLineActivity()
{
}

/**
 * Initiate this activity. Status should switch to INITIATED. Method should only work if the status was
 * UNDEFINED before. 
 */
void CommandLineActivity::initiateActivity() throw (ActivityException,StateTransitionException) {
	LOG_DEBUG(_logger, "initiateActivity(" << _id << ") ... ");
	//check status
	if (_status != STATUS_UNDEFINED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for initiating activity \"" << _id << "\"" << endl;
		throw StateTransitionException(oss.str());
	}
	// set working directory
	_workingDirectory.append(Utils::convertRelativeToAbsolutePath(GWES_TEMP_DIRECTORY));
	_workingDirectory.append("/");
	_workingDirectory.append(_id);
	// set stdoutfn
	_stdoutfn.append(_workingDirectory);
	_stdoutfn.append("/stdout.");
	_stdoutfn.append(_id);
	_stdoutfn.append(".dat");
	// set stderrfn
	_stderrfn.append(_workingDirectory);
	_stderrfn.append("/stderr.");
	_stderrfn.append(_id);
	_stderrfn.append(".dat");
	// set exitcodefn
	_exitcodefn.append(_workingDirectory);
	_exitcodefn.append("/exitcode.");
	_exitcodefn.append(_id);
	_exitcodefn.append(".dat");
	// set status
	setStatus(STATUS_INITIATED);
}

/**
 * Start this activity. Status should switch to RUNNING. 
 */
void CommandLineActivity::startActivity() throw (ActivityException,StateTransitionException,gwdl::WorkflowFormatException) {
	LOG_DEBUG(_logger, "startActivity(" << _id << ") ... ");
	//check status
	if (_status != STATUS_INITIATED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for starting activity \"" << _id << "\"" << endl;
		throw StateTransitionException(oss.str());
	}
	setStatus(STATUS_RUNNING);
	
	// create temp directory
	struct stat buffer;
	if (stat(GWES_TEMP_DIRECTORY, &buffer)!=0) {
		if (mkdir(GWES_TEMP_DIRECTORY, S_IRWXU)==0) LOG_INFO(_logger, "created directory " << GWES_TEMP_DIRECTORY);
		else {
			ostringstream oss;
			oss << "error creating " << GWES_TEMP_DIRECTORY << ": " << strerror(errno);
			LOG_ERROR(_logger, oss);
			throw ActivityException(oss.str());
		}
	} 
	if (mkdir(_workingDirectory.c_str(), S_IRWXU)==0) LOG_INFO(_logger, "created directory " << _workingDirectory);
	else {
		if (errno == EEXIST) LOG_WARN(_logger, "warning creating " << _workingDirectory << ": " << strerror(errno));
		else {
			ostringstream oss;
			oss << "error creating " << _workingDirectory << ": " << strerror(errno);
			LOG_ERROR(_logger, oss);
			throw ActivityException(oss.str());
		}
	}
	
	// generate command line
	ostringstream command;
	
	// change to temporary working directory
	command << "cd " << _workingDirectory << " && ";

	// executable
	command << Utils::expandEnv(_operation->getResourceName());

	//generate arguments from parameter tokens
	for (parameter_list_t::iterator it=_toP->tokens.begin(); it!=_toP->tokens.end(); ++it) {
		const string edgeExpression = it->edgeP->getExpression();
		if (!edgeExpression.empty() && edgeExpression.find("$")==edgeExpression.npos) {  // ignore XPath edge expressions
			switch (it->scope) {
			case (TokenParameter::SCOPE_READ):
			case (TokenParameter::SCOPE_INPUT):
				if (edgeExpression != "stdin") {
					if (it->tokenP->isData()) {
						string* textP = it->tokenP->getData()->getText();
						command << " -" << edgeExpression << " " << convertUrlToLocalPath(*textP);
					} else {
						command << " -" << edgeExpression << " " << (it->tokenP->getControl() ? "true" : "false");
					}
				}
			break;
			case (TokenParameter::SCOPE_WRITE):
				// do not convert edgeExpressions stdout, stderr, exitcode to command line parameters as they are handled differently.
				if (edgeExpression != "stdout" && edgeExpression != "stderr" && edgeExpression != "exitcode") {
					if (it->tokenP->isData()) {
						string* textP = it->tokenP->getData()->getText();
						command << " -" << edgeExpression << " " << convertUrlToLocalPath(*textP);
					} else {
						command << " -" << edgeExpression << " " << (it->tokenP->getControl() ? "true" : "false");
					}
				}
			break;
			case (TokenParameter::SCOPE_OUTPUT):	
				// do not convert edgeExpressions stdout, stderr, exitcode to command line parameters as they are handled differently.
				if (edgeExpression != "stdout" && edgeExpression != "stderr" && edgeExpression != "exitcode") {
					// create new data token for each output. 
					string url = generateOutputDataURL(edgeExpression);
					ostringstream oss;
					oss << "<data><" << edgeExpression << ">" << url << "</" << edgeExpression << "></data>";
					it->tokenP = new gwdl::Token(new gwdl::Data(oss.str()));
					command << " -" << edgeExpression << " " << convertUrlToLocalPath(url);
				}
			break;
			}
		}
	}
	
	// put additional simulation parameter
	if (_toP->simulation) {
		command << " -simulation " << convertUrlToLocalPath(generateOutputDataURL("simulation"));
	}

	///ToDo: redirect stdin to programm
	
	// redirect stdout to file
	command << " > " << _stdoutfn;
	// redirect stderr to file
	command << " 2> " << _stderrfn;
	
	// put exit code in file
	command << "; echo $? > " << _exitcodefn;
	
	// execute activity
    setStatus(STATUS_ACTIVE);
    string stdout = execute(command.str());
    setStatus(STATUS_RUNNING);
    // get exit code
    ifstream file(_exitcodefn.c_str());
    if (file.is_open()) {
        string line;
   		getline(file,line);
    	file.close();
    	char* pEnd;
    	_exitCode = strtol(line.c_str(),&pEnd,10);
    } else {
    	LOG_ERROR(_logger, "Unable to open file " << _exitcodefn << ": " << strerror(errno));
    	_exitCode = 255;
    }
    
	// notify observers
	if (_observers.size()>0) {
		ostringstream oss;
		oss << "exitcode=" << _exitCode;
		notifyObservers(Event::EVENT_ACTIVITY_END,oss.str(),&_toP->tokens);
	}

    (!_abort && _exitCode==0) ? setStatus(STATUS_COMPLETED) : setStatus(STATUS_TERMINATED);
}

string CommandLineActivity::generateOutputDataURL(const string& edgeExpression) {
	ostringstream oss;
	oss << "file://" << _workingDirectory << "/" << edgeExpression << "." << _id << ".dat";
    return oss.str();
}

string CommandLineActivity::convertUrlToLocalPath(const string& url) {
	string *ret = new string(url);
	string::size_type loc = ret->find("://",0);
	if (loc != string::npos) ret->erase(0,loc+3);
	return *ret;
}

/**
 * The "cd workingdirectory" must be part of the commandline, e.g., "cd test && execute..."
 */
string CommandLineActivity::execute(const string& commandline) {
    LOG_INFO(_logger, "startActivity(" << _id << ").execute(" << commandline << ")");
    // notify observers
	if (_observers.size()>0) {
		ostringstream oss;
		oss << "commandline=\"" << commandline << "\"";
		notifyObservers(Event::EVENT_ACTIVITY_START,oss.str(),&_toP->tokens);
	}
    string stdout;
    char buffer[1024];
    FILE* p=popen(commandline.c_str(), "r");
    while (fgets(buffer, 1024, p) != NULL) stdout.append(buffer);
    pclose(p);
	return stdout;
}

/**
 * Suspend this activity. Status should switch to SUSPENDED. 
 */
void CommandLineActivity::suspendActivity() throw (ActivityException,StateTransitionException) {
	LOG_INFO(_logger, "suspendActivity(" << _id << ") ... ");
	///ToDo: Implement!
	LOG_WARN(_logger, "suspendActivity() not yet implemented!");
}

/**
 * Resume this activity. Status should switch to RUNNING. 
 */
void CommandLineActivity::resumeActivity() throw (ActivityException,StateTransitionException) {
	LOG_INFO(_logger, "resumeActivity(" << _id << ") ... ");
	//check status
	if (_status != STATUS_SUSPENDED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for resuming activity \"" << _id << "\"" << endl;
		throw StateTransitionException(oss.str());
	}
	///ToDo: Implement!
	LOG_WARN(_logger, "resumeActivity() not yet implemented!");
}

/**
 * Abort this activity. Status should switch to TERMINATED.
 */
void CommandLineActivity::abortActivity() throw (ActivityException,StateTransitionException) {
	LOG_INFO(_logger, "abortActivity(" << _id << ") ... ");
	//check status
	if (_status == STATUS_COMPLETED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for aborting activity \"" << _id << "\"" << endl;
		throw StateTransitionException(oss.str());
	}

	_abort = true;
	waitForStatusChangeToCompletedOrTerminated();
	setStatus(STATUS_TERMINATED);
}

/**
 * Restart this activity. Status should switch to INITIATED. 
 */
void CommandLineActivity::restartActivity() throw (ActivityException,StateTransitionException) {
	LOG_INFO(_logger, "restartActivity(" << _id << ") ... ");
	///ToDo: Implement!
	LOG_WARN(_logger, "restartActivity() not yet implemented!");
}

} // end namespace gwes
