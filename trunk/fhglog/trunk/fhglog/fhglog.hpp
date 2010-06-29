#ifndef  FHGLOG_INC
#define  FHGLOG_INC

#include    <fhglog/fhglog-config.hpp>
#include    <fhglog/error_handler.hpp>
#include	<fhglog/LogMacros.hpp>
#include	<fhglog/LoggerApi.hpp>
#include	<fhglog/Appender.hpp>
#include    <fhglog/Filter.hpp>
#include    <fhglog/LogEvent.hpp>

#include	<fhglog/StreamAppender.hpp>
#include	<fhglog/SynchronizedAppender.hpp>
#include	<fhglog/FileAppender.hpp>
#include	<fhglog/CompoundAppender.hpp>

#if defined(FHGLOG_WITH_REMOTE_LOGGING)
#   include <fhglog/remote/RemoteAppender.hpp>
#endif

#include    <fhglog/Configuration.hpp>

#endif   /* ----- #ifndef FHGLOG_INC  ----- */
