/*
 * =====================================================================================
 *
 *       Filename:  StreamAppender.cpp
 *
 *    Description:  appending to a std::ostream
 *
 *        Version:  1.0
 *        Created:  08/25/2009 11:02:12 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include    "StreamAppender.hpp"

using namespace fhg::log;

void
StreamAppender::append(const LogEvent &evt) const
{
  const_cast<std::ostream&>(stream_) << getFormat()->format(evt);
}		/* -----  end of method ConsoleAppender::append  ----- */
