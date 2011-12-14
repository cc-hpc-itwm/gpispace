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
    "UNSET"
    , "TRACE"
    , "DEBUG"
    , "INFO"
    , "WARN"
    , "ERROR"
    , "FATAL"
  };
  assert(lvl_ >= 0 && lvl_ < static_cast<int>(sizeof(LevelToStringMap_)));
  return LevelToStringMap_[lvl_];
}
