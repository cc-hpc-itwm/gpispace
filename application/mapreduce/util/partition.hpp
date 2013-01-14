// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef _H_MAPREDUCE_UTIL_PAETITION
#define _H_MAPREDUCE_UTIL_PAETITION 1

#include <sstream>
#include <stdexcept>
#include <string>
#include <util/helper.hpp>

namespace mapreduce
{
  namespace util
  {
      int hash(const std::string& key, const int N)
      {
    	  if(N<3)
    	  {
    		  throw std::runtime_error("The number of partitions should be at least 3!");
    	  }

    	  int i = 0;
    	  char first_char = tolower(key[0]);

		  if( first_char>='a' && first_char<='z' )
		  {
			  i = ((first_char - 'a')*(N-2))/('z'-'a');
		  }
		  else
			  if(is_special_item(key))
				  i=N-1;
			  else
				  i=N-2;
        return i;
      }
  }
}

#endif
