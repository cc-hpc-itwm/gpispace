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

namespace fhg { namespace log {
  class FileAppender : public Appender
  {
  public:
    typedef shared_ptr<FileAppender> ptr_t;

    FileAppender( const std::string &a_name
                , const std::string &a_path
                , const std::string &fmt = default_format::LONG()
                , int flush_interval = 5
                , const std::ios_base::openmode &a_mode
                    = std::ios_base::out
                    | std::ios_base::app
                    | std::ios_base::binary
                );
    virtual ~FileAppender() throw();

    const std::string &path() const
    {
      return path_;
    }

    void set_path(const std::string &p)
    {
      path_ = p;
    }

    const std::ios_base::openmode &mode() const
    {
      return mode_;
    }

    void set_mode(const std::ios_base::openmode &a_mode)
    {
      mode_ = a_mode;
    }

    virtual void flush();
    virtual void close();
    virtual void open();
    virtual void reopen();

    virtual void append(const LogEvent &evt);
  private:
    std::string path_;
    std::ofstream stream_;
    std::string fmt_;
    std::ios_base::openmode mode_;
    int flush_interval_;
    mutable int event_count_;
  };
}}

#endif
