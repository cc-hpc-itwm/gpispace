#ifndef  FHGLOG_INC
#define  FHGLOG_INC

#if defined(HAVE_CONFIG_H)
#  include <fhglog/fhglog-config.hpp>
#endif

#include	<fhglog/LogMacros.hpp>
#include	<fhglog/LoggerApi.hpp>
#include	<fhglog/Appender.hpp>
#include    <fhglog/Filter.hpp>
#include    <fhglog/LogEvent.hpp>

#include	<fhglog/StreamAppender.hpp>
#include	<fhglog/SynchronizedAppender.hpp>
#include	<fhglog/FileAppender.hpp>

#if FHGLOG_WITH_REMOTE_LOGGING
#   include <fhglog/remote/RemoteAppender.hpp>
#endif

#include    <fhglog/Configuration.hpp>

#endif   /* ----- #ifndef FHGLOG_INC  ----- */
