/* 
   Copyright (C) 2009 Alexander Petry <alexander.petry@itwm.fraunhofer.de>.

   This file is part of seda.

   seda is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   seda is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with seda; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  

*/

#ifndef SEDA_LOGGING_HPP
#define SEDA_LOGGING_HPP 1

#include <seda/seda-config.hpp>

/* Logging */
#if defined(SEDA_ENABLE_LOGGING)

#include <fhglog/fhglog.hpp>

#define SEDA_LDECLARE_LOGGER(logger)           ::fhg::log::logger_t logger
#define SEDA_LDEFINE_LOGGER(logger, name) ::fhg::log::logger_t logger(SEDA_LINIT_LOGGER(logger, name))
#define SEDA_LINIT_LOGGER(logger, name)   logger(::fhg::log::getLogger(name))

#define SEDA_DECLARE_LOGGER()         SEDA_LDECLARE_LOGGER(seda_logger)
#define SEDA_DEFINE_LOGGER(hierarchy) SEDA_LDEFINE_LOGGER(seda_logger, hierarchy)
#define SEDA_INIT_LOGGER(hierarchy)   SEDA_LINIT_LOGGER(seda_logger, hierarchy)

#define SEDA_LLOG_DEBUG(logger, msg) LOG_DEBUG(logger, msg)
#define SEDA_LLOG_INFO(logger, msg)  LOG_INFO(logger, msg)
#define SEDA_LLOG_WARN(logger, msg)  LOG_WARN(logger, msg)
#define SEDA_LLOG_ERROR(logger, msg) LOG_ERROR(logger, msg)
#define SEDA_LLOG_FATAL(logger, msg) LOG_FATAL(logger, msg)

#define SEDA_LOG_DEBUG(msg) SEDA_LLOG_DEBUG(seda_logger, msg)
#define SEDA_LOG_INFO(msg)  SEDA_LLOG_INFO(seda_logger, msg)
#define SEDA_LOG_WARN(msg)  SEDA_LLOG_WARN(seda_logger, msg)
#define SEDA_LOG_ERROR(msg) SEDA_LLOG_ERROR(seda_logger, msg)
#define SEDA_LOG_FATAL(msg) SEDA_LLOG_FATAL(seda_logger, msg)

#else

#define SEDA_LDECLARE_LOGGER(logger)   void* __seda_unused_##logger
#define SEDA_LDEFINE_LOGGER(logger, h)
#define SEDA_LINIT_LOGGER(logger, h)   __seda_unused_##logger(0)

#define SEDA_DECLARE_LOGGER()          SEDA_LDECLARE_LOGGER(logger)
#define SEDA_DEFINE_LOGGER(hierarchy)  SEDA_LDEFINE_LOGGER(logger, hierarchy)
#define SEDA_INIT_LOGGER(hierarchy)    SEDA_LINIT_LOGGER(logger, hierarchy)

#define SEDA_LLOG_DEBUG(logger, msg)
#define SEDA_LLOG_INFO(logger, msg)
#define SEDA_LLOG_WARN(logger, msg)
#define SEDA_LLOG_ERROR(logger, msg)
#define SEDA_LLOG_FATAL(logger, msg)

#define SEDA_LOG_DEBUG(msg)
#define SEDA_LOG_INFO(msg)
#define SEDA_LOG_WARN(msg)
#define SEDA_LOG_ERROR(msg)
#define SEDA_LOG_FATAL(msg)

#endif

#endif // !SEDA_LOGGING_HPP
