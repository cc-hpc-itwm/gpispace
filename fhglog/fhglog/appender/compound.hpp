// alexander.petry@itwm.fraunhofer.de

#ifndef FHG_LOG_COMPOUND_APPENDER_HPP
#define FHG_LOG_COMPOUND_APPENDER_HPP 1

#include <fhglog/Appender.hpp>

#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

#include <list>

namespace fhg
{
  namespace log
  {
    class CompoundAppender : public Appender
    {
    public:
      typedef boost::shared_ptr<CompoundAppender> ptr_t;

      virtual void append (const LogEvent& evt)
      {
        BOOST_FOREACH (Appender::ptr_t const appender, _appender)
        {
          appender->append (evt);
        }
      }

      void addAppender (const Appender::ptr_t& a)
      {
        _appender.push_back (a);
      }

      virtual void flush()
      {
        BOOST_FOREACH (Appender::ptr_t const appender, _appender)
        {
          appender->flush();
        }
      }

    private:
      std::list<Appender::ptr_t> _appender;
    };
  }
}

#endif
