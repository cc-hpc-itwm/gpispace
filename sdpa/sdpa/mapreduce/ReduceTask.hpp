/*
 * =====================================================================================
 *
 *       Filename:  ReduceTask.hpp
 *
 *    Description:
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
#ifndef REDUCE_TASK_HPP
#define REDUCE_TASK_HPP

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

using namespace sdpa;
using namespace std;

template <typename InKey, typename InValue, typename OutValue>
class ReduceTask : public Task
{
public:

  typedef InKey InKeyT;
  typedef InValue InValueT;
  typedef OutValue OutValueT;

  typedef std::list<InValueT> ListInValuesT;
  typedef std::list<OutValueT> ListOutValuesT;

  ReduceTask(InKeyT inKey = InKeyT(), ListInValuesT listInValues = ListInValuesT())
  : inKey_(inKey),
    listInValues_(listInValues)
  {}

  ReduceTask(const InKeyT& inKey, const InValueT& inValue)
    : inKey_(inKey),
      listInValues_(1, inValue)
    {}

  virtual ~ReduceTask(){}

  template<class Archive>
  void serialize(Archive & ar, const unsigned int /* file_version */)
  {
    ar & boost::serialization::base_object<Task>(*this);
    ar & inKey_;
    ar & listInValues_;
    ar & listOutValues_;
  }

  void emit(const OutValue& outVal)
  {
	  listOutValues().push_back(outVal);
  }

  void run()
  {
    throw std::exception("Method not implemented!");
  }

  void accumulate(InValueT v)
  {
    listInValues_.push_back(v);
  }

  void clear() { listOutValues_.clear(); }

  void print()
  {
     std::cout<<"*******************ReduceTask*********************"<<std::endl;
     std::cout<<"inKey="<<inKey()<<std::endl;
     std::cout<<"listInValues: "<<std::endl;

     BOOST_FOREACH(InValueT& inv, listInValues_ )
       std::cout<<inv<<std::endl;

     std::cout<<"listOutValues: "<<std::endl;
     BOOST_FOREACH(InValueT& outv, listOutValues_ )
       std::cout<<outv<<std::endl;

     std::cout<<"**************************************************"<<std::endl;
  }

  void print(const std::string& strFileName)
  {
    std::ofstream ofs;
    ofs.open( strFileName.c_str(), ios::out | ios::app );

    typename ListOutValuesT::iterator lastItem(listOutValues_.end() );
    lastItem--;

    ofs<<"key:"<<inKey()<<", values: ";
    for(typename ListOutValuesT::iterator iter = listOutValues_.begin(); iter != lastItem; iter++ )
      ofs<<*iter<<", ";

    ofs<<*lastItem<<std::endl;
    ofs.close();
  }

  InKeyT inKey() { return inKey_; }
  ListInValuesT& listInValues() { return listInValues_; }
  ListOutValuesT& listOutValues() { return listOutValues_; }

protected:
  InKeyT inKey_;
  ListInValuesT listInValues_;
  ListOutValuesT listOutValues_;
};

#endif //REDUCE_TASK_HPP
