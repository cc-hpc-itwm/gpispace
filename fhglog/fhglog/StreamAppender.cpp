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

#include "StreamAppender.hpp"
#include "format.hpp"

using namespace fhg::log;

StreamAppender::StreamAppender( std::ostream &stream
                              , std::string const &fmt
                              , StreamAppender::ColorMode color_mode
                              )
  : stream_(stream)
  , fmt_(fmt)
  , color_mode_(color_mode)
{}

void
StreamAppender::append(const LogEvent &evt)
{
  if (color_mode_ == COLOR_ON)
    stream_ << color_map[(evt.severity())];

  // TODO: pass color mapper into the formatter
  stream_ << format(fmt_, evt);

  if (color_mode_ == COLOR_ON)
    stream_ << color_map.reset_escape_code();
}

void
StreamAppender::flush (void)
{
  stream_.flush();
}
