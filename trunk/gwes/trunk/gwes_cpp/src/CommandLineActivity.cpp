/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
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
//gwdl
#include "../../gworkflowdl_cpp/src/Data.h"
#include "../../gworkflowdl_cpp/src/Token.h"
//gwes
#include "CommandLineActivity.h"

using namespace std;

namespace gwes
{

CommandLineActivity::CommandLineActivity(WorkflowHandler* handler, gwdl::OperationCandidate* operation) : Activity(handler, "CommandLineActivity", operation)
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
	cout << "gwes::CommandLineActivity::initiateActivity(" << _id << ") ... " << endl;
	//check status
	if (_status != STATUS_UNDEFINED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for initiating activity \"" << _id << "\"" << endl;
		throw StateTransitionException(oss.str());
	}
	// set working directory
	_workingDirectory.append(GWES_TEMP_DIRECTORY);
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
	cout << "gwes::CommandLineActivity::startActivity(" << _id << ") ... " << endl;
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
		if (mkdir(GWES_TEMP_DIRECTORY, S_IRWXU)==0) cout << "gwes::CommandLineActivity::startActivity(): created directory " << GWES_TEMP_DIRECTORY << endl;
		else cerr << "gwes::CommandLineActivity::startActivity(): error creating " << GWES_TEMP_DIRECTORY << ": " << strerror(errno) << endl;
	} 
	if (mkdir(_workingDirectory.c_str(), S_IRWXU)==0) cout << "gwes::CommandLineActivity::startActivity(): created directory " << _workingDirectory << endl;
	else cerr << "gwes::CommandLineActivity::startActivity(): error creating " << _workingDirectory << ": " << strerror(errno) << endl;
	
	// generate command line
	ostringstream command;

	// executable
	command << _operation->getResourceName();

	//generate arguments from input tokens
	for (map<string,gwdl::Data*>::iterator it=_inputs.begin(); it !=_inputs.end(); ++it) {
		string edgeExpression = it->first;
		// stdin is reserved edgeExpression which is not treated as normal command line parameter. 
		if (edgeExpression != "stdin") {
			string text = *(it->second)->getText();
			command << " -" << edgeExpression << " " << convertUrlToLocalPath(text);
		}
	}
	
	//generate arguments from output tokens
	for (map<string,gwdl::Data*>::iterator it=_outputs.begin(); it != _outputs.end(); ++it) {
		string edgeExpression = it->first;
		// if edge expression of input and output place are the same then just copy the token from input to output. ///Where is this done?
		// if output edge expression is different from input edge expression, then create new data token. 
		if (_inputs.find(edgeExpression)==_inputs.end()) {
			string url = generateOutputDataURL(edgeExpression);
			ostringstream oss;
			oss << "<data><" << edgeExpression << ">" << url << "</" << edgeExpression << "></data>";
			gwdl::Data* data = new gwdl::Data(oss.str());
			// do not convert edgeExpressions stdout, stderr, exitcode to command line parameters as they are handled differently.
			if (edgeExpression != "stdout" && edgeExpression != "stderr" && edgeExpression != "exitcode") {
				command << " -" << edgeExpression << " " << convertUrlToLocalPath(url);
			}
			_outputs.erase(edgeExpression);
			_outputs.insert(pair<string,gwdl::Data*>(edgeExpression,data));
		}
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
    /// ToDo: put inputs and outputs
    // get exit code
    ifstream file(_exitcodefn.c_str());
    if (file.is_open()) {
        string line;
   		getline(file,line);
    	file.close();
    	char* pEnd;
    	_exitCode = strtol(line.c_str(),&pEnd,10);
    } else {
    	cerr << "Unable to open file " << _exitcodefn << ": " << strerror(errno) << endl;
    }
    
	// notify observers
	if (_observers.size()>0) {
		ostringstream oss;
		oss << "exitcode=" << _exitCode;
		notifyObservers(Event::EVENT_ACTIVITY_END,oss.str(),&_outputs);
	}

    (!_abort && _exitCode==0) ? setStatus(STATUS_COMPLETED) : setStatus(STATUS_TERMINATED);
}

string CommandLineActivity::generateOutputDataURL(string edgeExpression) {
	ostringstream oss;
	oss << "file://" << _workingDirectory << "/" << edgeExpression << "." << _id << ".dat";
    return oss.str();
}

string CommandLineActivity::convertUrlToLocalPath(string url) {
	string *ret = new string(url);
	string::size_type loc = ret->find("://",0);
	if (loc != string::npos) ret->erase(0,loc+3);
	return *ret;
}

/**
 * Note: execute is NOT thread safe, because of changing the working directory!
 * ToDo: make it thread safe.
 */
string CommandLineActivity::execute(string commandline) {
    cout << "gwes::CommandLineActivity::startActivity(" << _id << ").execute(" << commandline << ")" << endl;
    // notify observers
	if (_observers.size()>0) {
		ostringstream oss;
		oss << "commandline=\"" << commandline << "\"";
		notifyObservers(Event::EVENT_ACTIVITY_START,oss.str(),&_inputs);
	}
    // change to temporary working directory
	long size;
	char *buf;
	char *olddir;
	size = pathconf(".", _PC_PATH_MAX);
	if ((buf = (char *)malloc((size_t)size)) != NULL) {
	    olddir = getcwd(buf, (size_t)size);
	}
	//  char *getcwd(char *buf, size_t size);
    if (chdir(_workingDirectory.c_str())!=0) cerr << "error changing to working directory: " << strerror(errno) << endl;
    string stdout;
    char buffer[1024];
    FILE* p=popen(commandline.c_str(), "r");
    while (fgets(buffer, 1024, p) != NULL) stdout.append(buffer);
    pclose(p);
    // change back to old directory
    chdir(olddir);
    free(olddir);
	return stdout;
}

/**
 * Suspend this activity. Status should switch to SUSPENDED. 
 */
void CommandLineActivity::suspendActivity() throw (ActivityException,StateTransitionException) {
	cout << "gwes::CommandLineActivity::suspendActivity(" << _id << ") ... " << endl;
	///ToDo: Implement!
	cerr << "gwes::CommandLineActivity::suspendActivity() not yet implemented!" << endl;
}

/**
 * Resume this activity. Status should switch to RUNNING. 
 */
void CommandLineActivity::resumeActivity() throw (ActivityException,StateTransitionException) {
	cout << "gwes::CommandLineActivity::resumeActivity(" << _id << ") ... " << endl;
	//check status
	if (_status != STATUS_SUSPENDED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for resuming activity \"" << _id << "\"" << endl;
		throw StateTransitionException(oss.str());
	}
	///ToDo: Implement!
	cerr << "gwes::CommandLineActivity::resumeActivity() not yet implemented!" << endl;
}

/**
 * Abort this activity. Status should switch to TERMINATED.
 */
void CommandLineActivity::abortActivity() throw (ActivityException,StateTransitionException) {
	cout << "gwes::CommandLineActivity::abortActivity(" << _id << ") ... " << endl;
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
	cout << "gwes::CommandLineActivity::restartActivity(" << _id << ") ... " << endl;
	///ToDo: Implement!
	cerr << "gwes::CommandLineActivity::restartActivity() not yet implemented!" << endl;
}

} // end namespace gwes
