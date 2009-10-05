/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// gwes
#include <gwes/Utils.h>
//fhglog
#include <fhglog/fhglog.hpp>
// std
#include <stdlib.h>
#include <unistd.h>  // getcwd() definition
#include <sys/param.h>  // MAXPATHLEN definition
#include <iostream>
#include <sstream>

using namespace std;
using namespace gwes;
 
namespace gwes
{

bool Utils::endsWith(const string& s1, const string& s2) {
	if ( s2.size() > s1.size() ) return false;
	if ( s1.compare(s1.size()-s2.size(),s2.size(),s2 ) == 0) {
	   return true;
    }
	return false;
}

bool Utils::startsWith(const string& s1, const string& s2) {
	if ( s2.size() > s1.size() ) return false;
	if ( s1.compare(0,s2.size(),s2 ) == 0) {
	   return true;
    }
	return false;
}

string Utils::convertRelativeToAbsolutePath(const string& relpath) {
	// is already absolute path
	if (startsWith(relpath,"/")) return relpath;
	// get current working directory
	char pathC[MAXPATHLEN];
	getcwd(pathC, MAXPATHLEN);
	string path(pathC);
	path += "/";
	path += relpath;
	return path;
}

void Utils::setEnvironmentVariables() {
	static fhg::log::logger_t logger(fhg::log::getLogger("gwes"));
	const char* name = "GWES_CPP_HOME";
	const char* gwesHome = getenv(name);
	if (gwesHome == NULL) {
		// get current working directory
		char pathC[MAXPATHLEN];
		getcwd(pathC, MAXPATHLEN);
		string path(pathC);
		LOG_DEBUG(logger, "getcwd=" << path);
		string value;
		
		if ( Utils::endsWith(path,"/gwes/trunk") ) {
			value = path;
		} else if ( Utils::endsWith(path,"/build/tests") ) {
			value = path;
			value += "/../.."; 
		} else {
			value = path;
		}
		
		setenv(name,value.c_str(),1);
		LOG_DEBUG(logger, "setEnvironmentVariables(): setenv " << name << "=" << value);
	} else {
		LOG_DEBUG(logger, "setEnvironmentVariables(): " << name << "=" << gwesHome);
	}
}

string Utils::expandEnv(const string& path) {
	static fhg::log::logger_t logger(fhg::log::getLogger("gwes"));
	string::size_type loc1 = path.find("${",0);
	if (loc1!=path.npos) {
		string::size_type loc2 = path.find_first_of("}",loc1);
		if (loc2 != path.npos) {
			// get environment variable
			string name = path.substr(loc1+2,loc2-loc1-2);
			const char* value = getenv(name.c_str());
			if (value != NULL) {
				LOG_DEBUG(logger, "expanding ${" << name << "}=" << value);
				string* newPath = new string(path);
				return newPath->replace(loc1,loc2-loc1+1,value);
			}
		}
	}
	return path;
}



} // end namespace gwes
