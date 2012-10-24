// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef _H_MAPREDUCE_UTIL_REDUCE
#define _H_MAPREDUCE_UTIL_REDUCE 1

#include <sstream>
#include <stdexcept>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <sstream>
#include <list>

using namespace std;

namespace mapreduce
{
  namespace util
  {
    typedef std::pair<std::string, std::string> key_val_pair_t;
    typedef  std::list<key_val_pair_t> list_key_val_pairs_t;

    list_key_val_pairs_t map(const std::string& key, const std::string& val)
    {
       list_key_val_pairs_t list_key_val_pairs;
       list_key_val_pairs.push_back(key_val_pair_t(key,"1"));
       return list_key_val_pairs;
    }
  }
}

#endif
