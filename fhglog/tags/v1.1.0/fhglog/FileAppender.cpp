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

FileAppender::FileAppender(const std::string &a_name
                         , const std::string &a_path
                         , const std::ios_base::openmode &a_mode) throw (std::exception)
  : Appender(a_name)
  , path_(a_path)
  , stream_()
  , mode_(a_mode)
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

void FileAppender::flush() throw ()
{
  try
  {
    if (stream_.is_open()) stream_.flush();
  } catch (...)
  {
    // ignore
  }
}

void FileAppender::close() throw (std::exception)
{
  stream_.close();  
}

void FileAppender::open() throw (std::exception)
{
  stream_.open(path_.c_str(), mode_);
#ifndef NDEBUG // FIXME: use a better marking message
//  stream_ << "------ MARK (file opened)" << std::endl;
#endif
}

void FileAppender::reopen() throw (std::exception)
{
  flush();
  close();
  open();
}

void FileAppender::append(const LogEvent &evt) const
{
  const_cast<std::ofstream&>(stream_) << getFormat()->format(evt);
}
