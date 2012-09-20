/*
 * =====================================================================================
 *
 *       Filename:  Combiner.hpp
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
#ifndef COMBINER_HPP
#define COMBINER_HPP 1

#include <sdpa/mapreduce/Task.hpp>

template <typename Mapper, typename Reducer>
struct Combiner
{
  static void shuffle( typename Mapper::TaskT* pWcMapTask, Reducer& reducer)
  {
    BOOST_FOREACH(typename Mapper::TaskT::OutKeyValueMapT::value_type& pairWC, pWcMapTask->outKeyValueMap())
    {
      bool bFound = reducer.find(pairWC.first);

      if(bFound)
      {
        typename Reducer::TaskT& reduceTask = reducer[pairWC.first];
        reduceTask.accumulate(pairWC.second);
      }
      else
      {
        typename Reducer::TaskT reduceTask(pairWC.first, pairWC.second);
        reducer.addTask(pairWC.first, reduceTask);
      }
    }
  }
};

#endif //COMBINER_HPP
