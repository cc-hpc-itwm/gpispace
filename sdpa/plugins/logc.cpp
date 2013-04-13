#include "logc.hpp"

#include <fhglog/minimal.hpp>
#include <fhglog/ThreadedAppender.hpp>
#include <fhglog/remote/RemoteAppender.hpp>

#include <boost/function.hpp>

#include <fhglog/Appender.hpp>
#include <fhglog/fhglog.hpp>
#include <fhg/plugin/plugin.hpp>

class LogcPluginImpl;

static LogcPluginImpl *global_logc = 0;

class LogcPluginImpl : FHG_PLUGIN
{
public:
  LogcPluginImpl ()
  {
    global_logc = this;
  }

  FHG_PLUGIN_START()
  {
    m_url = fhg_kernel()->get("url", "");

    if ("" == m_url)
    {
      MLOG(ERROR, "no remote logging URL specified, please set plugin.logc.url");
      FHG_PLUGIN_FAILED(EINVAL);
    }

    try
    {
      m_destination.reset (new fhg::log::ThreadedAppender
                          (fhg::log::Appender::ptr_t
                          (new fhg::log::remote::RemoteAppender( "logc"
                                                               , m_url
                                                               )
                          )));
    }
    catch (std::exception const &ex)
    {
      MLOG(ERROR, "could not start appender to url: " << m_url << ": " << ex.what());
      FHG_PLUGIN_FAILED(EINVAL);
    }

    DMLOG(TRACE, "LOGC sending events to " << m_url);

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
    m_destination->append(fhg::log::LogEvent ( fhg::log::LogLevel::INFO
                                             , filename
                                             , function
                                             , line
                                             , message
                                             )
                         );
  }

private:
  std::string m_url;
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
