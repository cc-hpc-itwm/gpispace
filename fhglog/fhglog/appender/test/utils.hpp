// bernd.loerwald@itwm.fraunhofer.de

#include <fhglog/fhglog.hpp>

#include <fhg/util/counter.hpp>

#include <boost/format.hpp>

namespace utils
{
  namespace
  {
    fhg::util::counter<unsigned int> _logger_counter;
  }

  struct logger_with_minimum_log_level
  {
    logger_with_minimum_log_level()
      : log ( fhg::log::Logger::get
              ((boost::format ("logger-%1%") % _logger_counter.next()).str())
            )
    {
      log->setLevel (fhg::log::TRACE);
    }

    fhg::log::Logger::ptr_t log;
  };
}
