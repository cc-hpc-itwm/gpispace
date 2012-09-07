/*
 * =====================================================================================
 *
 *       Filename:  MapTask.hpp
 *
 *    Description:  Simulate simple gwes behavior
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#ifndef MAP_TASK_HPP
#define MAP_TASK_HPP

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include <sdpa/mapreduce/Task.hpp>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include <list>
#include <map>

using namespace sdpa;
using namespace std;

template <typename InKey, typename InValue, typename OutKey, typename OutValue>
class MapTask : public Task
{
public:

  typedef InKey InKeyT;
  typedef InValue InValueT;
  typedef OutKey OutKeyT;
  typedef OutValue OutValueT;

  typedef std::multimap<OutKeyT, OutValueT> OutKeyValueMapT;
  typedef std::pair<OutKeyT, OutValueT> OutKeyValPairT;

  MapTask(InKeyT inKey = InKeyT(), InValueT inVal = InValueT())
  : inKey_(inKey),
    inValue_(inVal)
  {}

  virtual ~MapTask(){}

  template<class Archive>
  void serialize(Archive & ar, const unsigned int /* file_version */)
  {
    ar & boost::serialization::base_object<Task>(*this);
    ar & inKey_;
    ar & inValue_;
    ar & outKeyValueMap_;
  }

  // run will produce outKeyValueMap_ !
  // example in word count inKey_ = filename, inValue_ = node
  // outKeyValueMap_ will be a map of <word, count> pairs
  void run() {}

  void emit(OutKeyT key, OutValueT val)
  {
	  outKeyValueMap_.insert(OutKeyValPairT(key,val));
  }

  InKeyT inKey() { return inKey_; }
  InValueT inValue() { return inValue_; }

  OutKeyValueMapT& outKeyValueMap() { return outKeyValueMap_; }

  void clear() { outKeyValueMap_.clear(); }

  void print()
  {
     std::cout<<"*******************MapTask***********************"<<std::endl;
     std::cout<<"inKey="<<inKey()<<std::endl;
     std::cout<<"inValue="<<inValue()<<std::endl;

     BOOST_FOREACH(OutKeyValPairT pair, outKeyValueMap_)
       std::cout<<"("<<pair.first<<", "<<pair.second<<")"<<std::endl;
     std::cout<<"**************************************************"<<std::endl;
  }

  void print(const std::string& strFileName) {}

private:
  InKeyT inKey_;
  InValueT inValue_;
  OutKeyValueMapT outKeyValueMap_;
};

#endif //MAP_TASK_HPP
