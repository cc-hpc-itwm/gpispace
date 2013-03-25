// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef _H_MAPREDUCE_UTIL_MAP
#define _H_MAPREDUCE_UTIL_MAP 1

#include <string>
#include <list>
#include <util/types.hpp>

using namespace std;

namespace mapreduce
{
  namespace util
  {
    list_key_val_pairs_t map(const std::string& key, const std::string& val)
    {
       list_key_val_pairs_t list_key_val_pairs;
       list_key_val_pairs.push_back(key_val_pair_t(key,"1"));
       return list_key_val_pairs;
    }
  }
}

#endif
