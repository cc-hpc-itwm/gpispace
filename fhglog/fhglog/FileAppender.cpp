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

#include "FileAppender.hpp"

#include <iostream> // std::clog
#include <exception>

using namespace fhg::log;

FileAppender::FileAppender( const std::string &a_name
                          , const std::string &a_path
                          , const std::string &fmt
                          , int flush_interval
                          , const std::ios_base::openmode &a_mode
                          )
  : Appender(a_name)
  , path_(a_path)
  , stream_()
  , fmt_(fmt)
  , mode_(a_mode)
  , flush_interval_(flush_interval)
  , event_count_(0)
{
  stream_.exceptions(std::ios_base::badbit | std::ios_base::failbit);
  open();
}

FileAppender::~FileAppender() throw ()
{
  try
  {
    if (stream_.is_open()) close();
  }
  catch (const std::exception &ex)
  {
    std::clog << "E: could not close ofstream: " << ex.what() << std::endl;
  }
  catch (...)
  {
    std::clog << "E: could not close ofstream (unknown error)!" << std::endl;
  }
}

void FileAppender::flush()
{
  try
  {
    stream_.flush();
  } catch (std::exception const & ex)
  {
    std::clog << "could not flush: " << ex.what();
  }
}

void FileAppender::close()
{
  stream_.close();
}

void FileAppender::open()
{
  stream_.open(path_.c_str(), mode_);
#ifndef NDEBUG // FIXME: use a better marking message
//  stream_ << "------ MARK (file opened)" << std::endl;
#endif
  event_count_ = 0;
}

void FileAppender::reopen()
{
  close();
  open();
}

void FileAppender::append(const LogEvent &evt)
{
  try
  {
    stream_ << format(fmt_, evt);
    if (++event_count_ >= flush_interval_)
    {
      flush();
      event_count_ = 0;
    }
  }
  catch (std::exception const & ex)
  {
    std::clog << "could not append to `" << path_ << "'" << std::endl;
    reopen();
  }
}
