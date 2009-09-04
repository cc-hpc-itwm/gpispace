/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// gwes
#include <gwes/Utils.h>
// std
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

void Utils::setEnvironmentVariables() {
	const char* name = "GWES_CPP_HOME";
	const char* gwesHome = getenv(name);
	if (gwesHome == NULL) {
		// get current working directory
		char pathC[MAXPATHLEN];
		getcwd(pathC, MAXPATHLEN);
		string path(pathC);
		cout << "Utils::setEnvironmentVariables(): getcwd=" << path << endl;
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
		cout << "Utils::setEnvironmentVariables(): setenv " << name << "=" << value << endl;

		//		// if /tmp/gwes/build_gwes_????
//		s2 = string("/tmp/gwes/build_gwes");
//		if ( startsWith(path,s2) ) {
//			return string("workflows");
//		}
//		
//		// default
//		cout << "CWD=" << path << endl;
//		return string ("../..");
	} else {
		cout << "Utils::setEnvironmentVariables(): " << name << "=" << gwesHome << endl;
	}
}

string Utils::expandEnv(const string& path) {
	string::size_type loc1 = path.find("${",0);
	if (loc1!=path.npos) {
		string::size_type loc2 = path.find_first_of("}",loc1);
		if (loc2 != path.npos) {
			// get environment variable
			string name = path.substr(loc1+2,loc2-loc1-2);
			const char* value = getenv(name.c_str());
			if (value != NULL) {
				cout << "gwes::Utils::expandEnv(): expanding ${" << name << "}=" << value << endl;
				string* newPath = new string(path);
				return newPath->replace(loc1,loc2-loc1+1,value);
			}
		}
	}
	return path;
}



} // end namespace gwes
