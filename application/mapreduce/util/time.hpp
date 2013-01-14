/*
 * time.hpp
 *
 *  Created on: Jan 14, 2013
 *      Author: rotaru
 */

#ifndef MAPREDUCE_TIME_HPP_
#define MAPREDUCE_TIME_HPP_

#include <sys/time.h>
#include <ctime>

using namespace std;

typedef unsigned long long timestamp_t;

namespace mapreduce
{
  namespace util
  {
	timestamp_t get_timestamp()
	{
		struct timeval now;
		gettimeofday (&now, NULL);
		return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
	}
  }
}

#endif /* TIME_HPP_ */
