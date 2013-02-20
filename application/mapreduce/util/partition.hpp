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
      int hash_1(const std::string& key, const int N)
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

      int hash_2(const std::string& key, const int N)
	  {
		 if(key.empty())
		 {
			 throw std::runtime_error("Invalid key (empty). Cannot compute the hash value!");
		 }

		  int i = 0;
		  int hash_val;
		  int K = 256;

		  if(key.size()==1)
			  hash_val = tolower(key[0])*K+tolower(key[0]);
		  else
			  hash_val = tolower(key[0])*K+tolower(key[1]);

		  int low = 'a'*K+'a';
		  int high = 'z'*K+'z';

		  if( hash_val>= low && hash_val<=high )
		  {
			  i = ((hash_val - low)*(N-2))/(high-low);
		  }
		  else
			  if(is_special_item(key))
				  i=N-1;
			  else
				  i=N-2;
		 return i;
	  }

      int hash(const std::string& key, const int N)
	  {
    	  if(N<=28)
    		  return hash_1(key, N);
    	  else
    		  return hash_2(key, N);
	  }
  }
}

#endif
