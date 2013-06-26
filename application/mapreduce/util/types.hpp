// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef _H_MAPREDUCE_UTIL_TYPES
#define _H_MAPREDUCE_UTIL_TYPES 1

#include <sstream>
#include <string>
#include <sstream>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

using namespace std;

namespace mapreduce
{
  namespace util
  {
    typedef std::pair<std::string, std::string> key_val_pair_t;
    typedef std::list<key_val_pair_t> list_key_val_pairs_t;
    typedef std::vector<std::string> set_of_mapped_items_t;
  }
}

#endif
