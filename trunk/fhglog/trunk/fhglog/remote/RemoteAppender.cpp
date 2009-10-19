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

RemoteAppender::RemoteAppender(const std::string &name, const std::string &, uint16_t port)
  : Appender(name)
  {

  }

RemoteAppender::~RemoteAppender()
{
}
