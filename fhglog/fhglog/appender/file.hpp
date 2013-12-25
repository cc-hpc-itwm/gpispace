/*
 * =====================================================================================
 *
 *       Filename:  FileAppender.hpp
 *
 *    Description:  appends to files
 *
 *        Version:  1.0
 *        Created:  10/09/2009 11:24:48 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef FHG_LOG_FILE_APPENDER_HPP
#define FHG_LOG_FILE_APPENDER_HPP 1

#include <string>
#include <fstream>
#include <stdexcept>

#include <fhglog/format.hpp>
#include <fhglog/Appender.hpp>

#include <boost/shared_ptr.hpp>

namespace fhg { namespace log {
  class FileAppender : public Appender
  {
  public:
    typedef boost::shared_ptr<FileAppender> ptr_t;

    FileAppender( const std::string &a_path
                , const std::string &fmt
                , int flush_interval = 5
                , const std::ios_base::openmode &a_mode
                    = std::ios_base::out
                    | std::ios_base::app
                    | std::ios_base::binary
                );

    virtual void flush();
    virtual void append(const LogEvent &evt);

  private:
    std::ofstream stream_;
    std::string const fmt_;
    int const flush_interval_;
    int event_count_;
  };
}}

#endif
