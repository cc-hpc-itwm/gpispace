#include "log.hpp"

#include <fhglog/fhglog.hpp>

namespace gspc
{
  namespace ctl
  {
    template <int CAT=DEFAULT> struct get_category_name
    { static const char *name () { return "default"; } };

    template <> struct get_category_name<APPLICATION>
    { static const char *name () { return "application"; } };
    template <> struct get_category_name<NETWORK>
    { static const char *name () { return "network"; } };
    template <> struct get_category_name<AGENT>
    { static const char *name () { return "agent"; } };
    template <> struct get_category_name<ENGINE>
    { static const char *name () { return "engine"; } };
    template <> struct get_category_name<WORKER>
    { static const char *name () { return "worker"; } };
    template <> struct get_category_name<SYSTEM>
    { static const char *name () { return "system"; } };
    template <> struct get_category_name<USER>
    { static const char *name () { return "user"; } };

    template <int CAT=DEFAULT>
    struct get_logger
    {
      static void log (fhg::log::LogEvent const &evt)
      {
        static fhg::log::Logger::ptr_t
          logger (fhg::log::Logger::get (get_category_name<CAT>::name ()));
        logger->log (evt);
      }
    };

    struct fhglog_initializer
    {
      fhglog_initializer ()
      {
        FHGLOG_SETUP ();
      }
    };

    void log ( log_category_t cat
             , const char *tag
             , int level
             , const char *file
             , const char *function
             , int line
             , const char *message
             )
    {
      using namespace fhg::log;

      static fhglog_initializer __fhglog_initializer;

      if (level < LogLevel::MIN_LEVEL || level > LogLevel::MAX_LEVEL)
        level = LogLevel::DEF_LEVEL;
      LogEvent evt ( LogLevel ((LogLevel::Level) level)
                   , file
                   , function
                   , line
                   , message
                   );
      evt.tag (tag);

      switch (cat)
      {
      case APPLICATION:
        get_logger<APPLICATION>::log (evt);
        break;
      case NETWORK:
        get_logger<NETWORK>::log (evt);
        break;
      case AGENT:
        get_logger<AGENT>::log (evt);
        break;
      case ENGINE:
        get_logger<ENGINE>::log (evt);
        break;
      case WORKER:
        get_logger<WORKER>::log (evt);
        break;
      case SYSTEM:
        get_logger<SYSTEM>::log (evt);
        break;
      case USER:
        get_logger<USER>::log (evt);
        break;
      default:
        get_logger<>::log (evt);
        break;
      }
    }
  }
}
