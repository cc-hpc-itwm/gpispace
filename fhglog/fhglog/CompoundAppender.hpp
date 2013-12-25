/*
 * =====================================================================================
 *
 *       Filename:  CompoundAppender.hpp
 *
 *    Description:  an appender that contains several appenders
 *
 *        Version:  1.0
 *        Created:  10/13/2009 12:44:05 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef FHG_LOG_COMPOUND_APPENDER_HPP
#define FHG_LOG_COMPOUND_APPENDER_HPP 1

#include <list>
#include <fhglog/Appender.hpp>

#include <boost/shared_ptr.hpp>

namespace fhg { namespace log {
  class CompoundAppender : public Appender
  {
	private:
	  typedef std::list<Appender::ptr_t> appender_list;
    public:
    typedef boost::shared_ptr<CompoundAppender> ptr_t;

      virtual void append(const LogEvent &evt)
      {
		for (appender_list::iterator a(appenders_.begin()); a != appenders_.end(); ++a)
		{
		  (*a)->append(evt);
		}
      }

      virtual const Appender::ptr_t &addAppender(const Appender::ptr_t &a)
	  {
		appenders_.push_back(a);
		return a;
	  }

	  virtual void clear()
	  {
		appenders_.clear();
	  }

    virtual void flush(void)
    {
      for (appender_list::iterator a(appenders_.begin()); a != appenders_.end(); ++a)
      {
        (*a)->flush();
      }
    }
    private:
	  appender_list appenders_;
  };
}}

#endif
