/*
 * =====================================================================================
 *
 *       Filename:  FileAppender.cpp
 *
 *    Description:  implements the file appender
 *
 *        Version:  1.0
 *        Created:  10/10/2009 02:36:10 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include "file.hpp"

#include <iostream> // std::clog
#include <exception>

using namespace fhg::log;

FileAppender::FileAppender( const std::string &a_path
                          , const std::string &fmt
                          , int flush_interval
                          , const std::ios_base::openmode &a_mode
                          )
  : stream_(a_path.c_str(), a_mode)
  , fmt_(fmt)
  , flush_interval_(flush_interval)
  , event_count_(0)
{
  stream_.exceptions(std::ios_base::badbit | std::ios_base::failbit);
}

void FileAppender::flush()
{
  stream_.flush();
}

void FileAppender::append(const LogEvent &evt)
{
  stream_ << format(fmt_, evt);

  if (++event_count_ >= flush_interval_)
  {
    flush();
    event_count_ = 0;
  }
}
