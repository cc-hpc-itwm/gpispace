/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef UTILS_H_
#define UTILS_H_
// std
#include <string> 

namespace gwes
{

class Utils
{

public:

	static bool endsWith(const std::string& s1, const std::string& s2);

	static bool startsWith(const std::string& s1, const std::string& s2);

	/**
	 * Sets GWES_CPP_HOME if not set yet.
	 */
	static void setEnvironmentVariables();
	
	static std::string expandEnv(const std::string& path);

}; // end class Utils

} // end namespace gwes
	 
#endif /*UTILS_H_*/
