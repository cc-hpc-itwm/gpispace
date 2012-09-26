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

  MapReduce(const InKeyT& inKey = InKeyT(), const InValueT& inVal = InValueT(), const MapKeyValueT& inputMapKeyVal = MapKeyValueT())
  : inKey_(inKey),
    inVal_(inVal),
    nCounter_(0)
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
      std::cout<<"Task to be assigned to the worker "<<pairInKeyTask.first<<std::endl;
      pairInKeyTask.second.print();
    }
  }

  bool find(const InKeyT& key)
  {
    return (mapOfTasks_.find(key) != mapOfTasks_.end());
  }

  bool empty() { return mapOfTasks_.empty(); }

  MapOfTasksT& mapOfTasks() { return mapOfTasks_; }

  std::string hash(const InKeyT& key, const std::vector<std::string>& workerIdList )
  {
    // to be specialized
    return "";
  }

  /* create a map structure, mapping the workerId to the tasks
  * thare are to be assigned to it
 */
  void partitionate(TaskT& mapTask, std::vector<std::string>& workerIdList )
  {
    // initialize the array of map tasks
    BOOST_FOREACH(const std::string& destWorkerId, workerIdList)
    {
      TaskT task(destWorkerId, mapTask.inValue());
      addTask(destWorkerId, task);
    }

    // now, just distribute the list of pairs [<word, count>] from mapTask
    // across the ẃorkers (create the corresponding buckets (tasks to be assigned)
    BOOST_FOREACH(const typename TaskT::OutKeyValPairT& pairKeyVal, mapTask.outKeyValueMap() )
    {
      size_t nWorkers = workerIdList.size();

      // map <word, count> -> bucket
      InKeyT key = pairKeyVal.first;
      std::string workerId = hash(key, workerIdList);

      mapOfTasks_[workerId].emit(key, pairKeyVal.second);
    }
  }

  // collect the reduced pairs from pReducer_ into the mapTask
  template <typename MapTaskT>
  void collect(MapTaskT& mapTask)
  {
      BOOST_FOREACH(typename MapOfTasksT::value_type& pairKeyTask, mapOfTasks())
      {
        id_type tag(mapTask.inValue());
        TaskT reduceTask = pairKeyTask.second;
        reduceTask.run();
        typename MapTaskT::OutValueT outVal(reduceTask.listOutValues().front());
        mapTask.emit(pairKeyTask.first, outVal);
      }
  }

  template<class Archive>
  void serialize(Archive & ar, const unsigned int /* file_version */)
  {
    ar & mapOfTasks_;
  }

  void clear()
  {
    mapOfTasks_.clear();
    resetCounter();
  }

  void updateCounter() { nCounter_++; }
  void resetCounter() { nCounter_ = 0; }

  bool isBoundReached(const int N ) { return nCounter_ == N; }

  void setId(const std::string& id) { id_ = id; }
  std::string id() { return id_; }

  std::string encode()
  {
    std::stringstream osstr;
    boost::archive::text_oarchive ar(osstr);
    ar << *this;
    return osstr.str();
  }

  void decode(const std::string& strWorkflow)
  {
    std::stringstream sstr(strWorkflow);
    boost::archive::text_iarchive ar(sstr);
    ar >> *this;
  }

private:
  InKeyT inKey_;
  InValueT inVal_;
  MapOfTasksT mapOfTasks_;
  int nCounter_;
  std::string id_;
};



#endif //MAP_REDUCE_HPP
