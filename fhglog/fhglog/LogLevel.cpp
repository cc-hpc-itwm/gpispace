/*
 * =====================================================================================
 *
 *       Filename:  LogLevel.cpp
 *
 *    Description:  implementation of log severities
 *
 *        Version:  1.0
 *        Created:  08/11/2009 05:27:17 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */


#include    <cassert>
#include	"LogLevel.hpp"

using namespace fhg::log;

const std::string &LogLevel::str() const
{
  static std::string LevelToStringMap_[] =
  {
      "TRACE"
    , "DEBUG"
    , "INFO"
    , "WARN"
    , "ERROR"
    , "FATAL"
  };
  assert(lvl_ >= 0 && lvl_ < 6);
  return LevelToStringMap_[lvl_];
}

LogLevel::LogLevel(const std::string &name)
{
  // TODO: make this better
  if      (name == "TRACE") lvl_ = TRACE;
  else if (name == "DEBUG") lvl_ = DEBUG;
  else if (name == "INFO")  lvl_ = INFO;
  else if (name == "WARN")  lvl_ = WARN;
  else if (name == "ERROR") lvl_ = ERROR;
  else if (name == "FATAL") lvl_ = ERROR;
  else if (name == "MIN")   lvl_ = MIN_LEVEL;
  else if (name == "MAX")   lvl_ = MAX_LEVEL;
  else if (name == "DEF")   lvl_ = DEF_LEVEL;
  else                      lvl_ = DEF_LEVEL;
}
