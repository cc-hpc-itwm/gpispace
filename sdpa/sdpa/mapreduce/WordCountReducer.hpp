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
  unsigned int sum = 0;
  BOOST_FOREACH(const unsigned int& val, listInValues())
  {
    sum += val;
  }

  //listOutValues().push_back(sum);
  emit(sum);
}

typedef MapReduce<WordCountReduceTask> WordCountReducer;

#endif //WORD_COUNT_REDUCER_HPP
