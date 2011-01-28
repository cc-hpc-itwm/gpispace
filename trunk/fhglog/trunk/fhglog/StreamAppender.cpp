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
#include "fileno.hpp" // fileno for streams
#include "color_map.hpp"
#include "unistd.h"   // isatty

using namespace fhg::log;

static color_map_t & get_color_map()
{
  static color_map_t color_map;
  return color_map;
}

StreamAppender::StreamAppender( const std::string &a_name
                              , std::ostream &stream
                              , std::string const &fmt
                              , StreamAppender::ColorMode color_mode
                              )
  : Appender(a_name), stream_(stream), fmt_(fmt), color_mode_(color_mode)
{
  if (color_mode_ == COLOR_AUTO)
  {
    int fd (fileno (stream));
    if (fd < 0)
    {
      color_mode_ = COLOR_OFF;
    } else if (isatty(fd))
    {
      color_mode_ = COLOR_ON;
    }
    else
    {
      color_mode_ = COLOR_OFF;
    }
  }
}

void
StreamAppender::append(const LogEvent &evt)
{
  if (color_mode_ == COLOR_ON)
    stream_ << get_color_map()[(evt.severity())];

  // TODO: pass color mapper into the formatter
  stream_ << format(fmt_, evt);

  if (color_mode_ == COLOR_ON)
    stream_ << get_color_map().reset_escape_code();
}

void
StreamAppender::flush (void)
{
  stream_.flush();
}
