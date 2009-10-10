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

#include <fhglog/Appender.hpp>


namespace fhg { namespace log {
  class FileAppender : public Appender
  {
  public:
    typedef shared_ptr<FileAppender> ptr_t;

    FileAppender(const std::string &a_name
               , const std::string &a_path) throw (std::exception);
    virtual ~FileAppender() throw();

    const std::string &path() const
    {
      return path_;
    }

    void set_path(const std::string &p)
    {
      path_ = p;
    }

    virtual void flush() throw ();
    virtual void close() throw (std::exception);
    virtual void open() throw (std::exception);
    virtual void reopen() throw (std::exception);

    virtual void append(const LogEvent &evt) const;
  private:
    std::string path_;
    std::ofstream stream_;
  };
}}

#endif
