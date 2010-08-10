/*
 * =====================================================================================
 *
 *       Filename:  SyslogAppender.cpp
 *
 *    Description:  implements syslog appender
 *
 *        Version:  1.0
 *        Created:  10/18/2009 01:29:41 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include "SyslogAppender.hpp"
#include <fhglog/format.hpp>
using namespace fhg::log;

void SyslogAppender::append(const fhg::log::LogEvent &evt)
{
  syslog(evt.severity().lvl(), "%s", format(fmt_, evt).c_str());
}
