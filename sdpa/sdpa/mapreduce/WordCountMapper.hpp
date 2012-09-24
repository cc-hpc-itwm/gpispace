/*
 * =====================================================================================
 *
 *       Filename:  WordCountMapper.hpp
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
#ifndef WORD_COUNT_MAPPER_HPP
#define WORD_COUNT_MAPPER_HPP

#include <sdpa/mapreduce/MapReduce.hpp>
#include <sdpa/mapreduce/MapTask.hpp>

#include <iostream>
#include <string>


typedef MapTask<std::string, std::string, std::string, unsigned int> WordCountMapTask;

template <>
inline void WordCountMapTask::run()
{
  // assume inKey_ is the absolute file name
  // and inValue_ is the node's name

  std::ifstream ifs(inKey().c_str());

  if( !ifs.good() )
  {
    std::cout<<"Could not find the file "<<inKey()<<"!"<<std::endl;
    return;
  }

  std::string line;
  while( std::getline(ifs, line) )
  {
   // break each line into words, according to problem spec
    istringstream sstr(line);
    string s;
    while(sstr>>s)
      emit(s,1);
  }

  ifs.close();
}

typedef MapReduce<WordCountMapTask> WordCountMapper;

template <>
inline std::string WordCountMapper::hash(const std::string& key, const std::vector<std::string>& workerIdList )
{
  size_t nWorkers = workerIdList.size();
  int i = 0;

  if((key[0]>=65 && key[0]<=90) )
  {
    i = (key[0] - 'a')%nWorkers;
  }
  else
  if( key[0]>=97 && key[0]<=122 )
  {
    i = (key[0] - 'A')%nWorkers;
  }

  // it is guaranteed that at least one worker is selected -> workerIdList[0]
  return workerIdList[i];
}

#endif //WORD_COUNT_MAPPER_HPP
