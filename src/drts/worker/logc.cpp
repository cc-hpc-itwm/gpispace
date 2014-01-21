#include "logc.hpp"

#include <fhglog/LogMacros.hpp>
#include <fhglog/remote/appender.hpp>

#include <boost/function.hpp>

#include <fhglog/Appender.hpp>
#include <fhglog/fhglog.hpp>
#include <fhg/plugin/plugin.hpp>

class LogcPluginImpl;

namespace
{
  LogcPluginImpl *global_logc = 0;
}

class LogcPluginImpl : FHG_PLUGIN
{
public:
  LogcPluginImpl ()
  {
    global_logc = this;
  }
  ~LogcPluginImpl()
  {
    global_logc = NULL;
  }

  FHG_PLUGIN_START()
  {
    std::string logc_url (fhg_kernel()->get("url", ""));

    if (logc_url.empty())
    {
      MLOG(ERROR, "no remote logging URL specified, please set plugin.logc.url");
      FHG_PLUGIN_FAILED(EINVAL);
    }

    try
    {
      m_destination.reset (new fhg::log::remote::RemoteAppender(logc_url));
    }
    catch (std::exception const &ex)
    {
      MLOG(ERROR, "could not start appender to url: " << logc_url << ": " << ex.what());
      FHG_PLUGIN_FAILED(EINVAL);
    }

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    FHG_PLUGIN_STOPPED();
  }

  void log ( const char * filename
           , const char * function
           , const size_t line
           , const char * message
           )
  {
    m_destination->append(fhg::log::LogEvent ( fhg::log::INFO
                                             , filename
                                             , function
                                             , line
                                             , message
                                             )
                         );
  }

private:
  fhg::log::Appender::ptr_t m_destination;
};

void fhg_emit_log_message ( const char *filename
                          , const char *function
                          , size_t line
                          , const char * msg
                          )
{
  if (global_logc)
    global_logc->log (filename, function, line, msg);
}

EXPORT_FHG_PLUGIN( logc
                 , LogcPluginImpl
                 , ""
                 , "provides access to the logging client"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
