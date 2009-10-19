/*
 * =====================================================================================
 *
 *       Filename:  RemoteAppender.cpp
 *
 *    Description:  implementation of the remote appender
 *
 *        Version:  1.0
 *        Created:  10/19/2009 02:38:48 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <fhglog/remote/RemoteAppender.hpp>

using namespace fhg::log::remote;

RemoteAppender::RemoteAppender(const std::string &a_name, const std::string &a_host, uint16_t a_port)
  : Appender(a_name)
  , host_(a_host)
  , port_(a_port)
  {

  }

RemoteAppender::~RemoteAppender()
{
}

void RemoteAppender::append(const LogEvent &/*evt*/) const
{
  // serialize event and send it
}
