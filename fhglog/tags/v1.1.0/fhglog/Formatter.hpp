#ifndef  FHG_LOG_FORMATTER_HPP_INC
#define  FHG_LOG_FORMATTER_HPP_INC

#include <fhglog/memory.hpp>
#include <fhglog/LogEvent.hpp>

namespace fhg { namespace log {
  /*
   * =====================================================================================
   *        Class:  Formatter
   *  Description:  formats a LogEvent to a string
   * =====================================================================================
   */
  class Formatter
  {
    public:
      typedef shared_ptr<Formatter> ptr_t;

      static const char FMT_SHORT_SEVERITY  = 's'; // the level of the event
      static const char FMT_SEVERITY  = 'S'; // the level of the event
      static const char FMT_FILE      = 'p'; // just the filename
      static const char FMT_PATH      = 'P'; // complete path of the file
      static const char FMT_FUNCTION  = 'F'; // function (full signature)
      static const char FMT_LINE      = 'L'; // line of the log
      static const char FMT_MESSAGE   = 'm'; // the logmessage
      static const char FMT_TIMESTAMP = 't'; // when was the event created
      static const char FMT_TID       = 'T'; // by which thread has it been created
      static const char FMT_PID       = 'R'; // by which process has it been created
      static const char FMT_NEWLINE   = 'n'; // a line separator
      static const char FMT_LOGGER    = 'l'; // the logger via which this event came

      virtual ~Formatter() {}

      static ptr_t Custom(const std::string &fmt)
      {
        return ptr_t(new Formatter(fmt));
      }
      static ptr_t Default()
      {
        return Short();
      }
      static ptr_t Full()
      {
        return Custom("%t %S %l pid:%R thread:%T %p:%L (%F) - %m%n");
      }
      static ptr_t Short()
      {
        return Custom("%s: %l %p:%L - %m%n");
      }

      virtual std::string format(const LogEvent &evt);
      std::string operator()(const LogEvent &evt) { return format(evt); }
      void setFormat(const std::string &fmt) { fmt_ = fmt; }
    private:
      explicit
      Formatter(const std::string &fmt) : fmt_(fmt) {}

      std::string fmt_;
  }; /* -----  end of class Formatter  ----- */
}}

#endif   /* ----- #ifndef FHG_LOG_FORMATTER_HPP_INC  ----- */
