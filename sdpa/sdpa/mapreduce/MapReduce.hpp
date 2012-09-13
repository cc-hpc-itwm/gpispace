/*
 * =====================================================================================
 *
 *       Filename:  MapReduce.hpp
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
#ifndef MAP_REDUCE_HPP
#define MAP_REDUCE_HPP 1

#include <sdpa/mapreduce/Task.hpp>

template <typename T>
class MapReduce
{
public:
  typedef typename T::InKeyT InKeyT;
  typedef typename T::InValueT InValueT;
  typedef typename T::OutValueT OutValueT;
  typedef T TaskT;

  typedef std::map<InKeyT, T> MapOfTasksT;
  typedef std::map<InKeyT, InValueT> MapKeyValueT;

  MapReduce(const MapKeyValueT& inputMapKeyVal = MapKeyValueT())
  {
    BOOST_FOREACH(const typename MapKeyValueT::value_type& pairKeyVal, inputMapKeyVal)
    {
      T task(pairKeyVal.first, pairKeyVal.second);
      addTask(pairKeyVal.first, task);
    }
  }

  void addTask(const InKeyT& inKey, const T& mapTask)
  {
    mapOfTasks_.insert(typename MapOfTasksT::value_type(inKey, mapTask));
  }

  void deleteTask(const InKeyT& inKey)
  {
    mapOfTasks_.erase(inKey);
  }

  T& operator[](const InKeyT& key)
  {
    try {
      return mapOfTasks_[key];
    }
    catch(exception& exc)
    {
      std::cout<<"Exception occurred! No task found associated tothe key "<<key<<std::endl;
      throw(exc);
    }
  }

  void print()
  {
    BOOST_FOREACH(typename MapOfTasksT::value_type& pairInKeyTask, mapOfTasks_)
    {
      pairInKeyTask.second.print();
    }
  }

  bool find(const InKeyT& key)
  {
    return (mapOfTasks_.find(key) != mapOfTasks_.end());
  }

  bool empty() { return mapOfTasks_.empty(); }

  MapOfTasksT& mapOfTasks() { return mapOfTasks_; }

private:
  MapOfTasksT mapOfTasks_;
};



#endif //MAP_REDUCE_HPP
