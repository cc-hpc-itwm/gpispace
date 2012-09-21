/*
 * =====================================================================================
 *
 *       Filename:  WordCountReducer.hpp
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
#ifndef WORD_COUNT_REDUCER_HPP
#define WORD_COUNT_REDUCER_HPP

#include <sdpa/mapreduce/MapReduce.hpp>
#include <sdpa/mapreduce/ReduceTask.hpp>

typedef ReduceTask< std::string, unsigned int, unsigned int> WordCountReduceTask;

template <>
inline void ReduceTask< std::string, unsigned int, unsigned int>::run()
{
  listOutValues_.clear();
  unsigned int sum = 0;
  BOOST_FOREACH(const unsigned int& val, listInValues())
  {
    sum += val;
  }

  emit(sum);
}

typedef MapReduce<WordCountReduceTask> WordCountReducer;

// collect the reduced pairs from pReducer_ into the mapTask
template<>
template <typename M>
void WordCountReducer::collect(M& mapTask)
{
    BOOST_FOREACH(WordCountReducer::MapOfTasksT::value_type& pairKeyTask, mapOfTasks())
    {
      id_type tag(mapTask.inValue());
      TaskT reduceTask = pairKeyTask.second;
      reduceTask.run();
      typename M::OutValueT outVal(reduceTask.listOutValues().front());
      mapTask.emit(pairKeyTask.first, outVal);
    }
}

#endif //WORD_COUNT_REDUCER_HPP
