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

        if((key[0]>='A' && key[0]<='Z') )
        {
          i = (key[0] - 'A')/N;
        }
        else
        if( key[0]>='a' && key[0]<='z' )
        {
          i = (key[0] - 'a')/N;
        }
        else
          i=N-1;

        return i;
      }
  }
}

#endif
