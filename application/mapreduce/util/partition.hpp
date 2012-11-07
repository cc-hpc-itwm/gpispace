// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef _H_MAPREDUCE_UTIL_PAETITION
#define _H_MAPREDUCE_UTIL_PAETITION 1

#include <sstream>
#include <stdexcept>
#include <string>

namespace mapreduce
{
  namespace util
  {
      int hash(const std::string& key, const int N)
      {
        int i = 0;
        char first_char = key[0];

        if((first_char>='A' && first_char<='Z') )
        {
          i = ((key[0] - 'A')*(N-1))/('Z'-'A');
        }
        else
        if( first_char>='a' && first_char<='z' )
        {
          i = ((key[0] - 'a')*(N-1))/('z'-'a');
        }
        else
          i=N-1;

        return i;
      }
  }
}

#endif
