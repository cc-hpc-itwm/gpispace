/*
  public domain code taken from:
       http://www.ginac.de/~kreckel/fileno/

       Credits go to: Richard B. Kreckel
 */

#ifndef FHGLOG_FILENO_HACK_HPP
#define FHGLOG_FILENO_HACK_HPP 1

#include <iosfwd>

template <typename charT, typename traits>
int fileno(const std::basic_ios<charT, traits>& stream);

#endif
