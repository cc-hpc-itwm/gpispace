#ifndef  FHG_LOG_FORMATTER_HPP_INC
#define  FHG_LOG_FORMATTER_HPP_INC

#include <tr1/memory>
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
      typedef std::tr1::shared_ptr<Formatter> ptr_t;

      static const std::string FMT_SEVERITY;  // = "S";
      static const std::string FMT_FILE;      // = "f";
      static const std::string FMT_FUNCTION;  // = "F";
      static const std::string FMT_LINE;      // = "L";
      static const std::string FMT_MESSAGE;   // = "m";
      static const std::string FMT_TIMESTAMP; // = "t";
      static const std::string FMT_THREAD;    // = "T";
      static const std::string FMT_NEWLINE;   // = "n";

      explicit
      Formatter(const std::string &fmt) : fmt_(fmt) {}
      virtual ~Formatter() {}

      static ptr_t DefaultFormatter()
      {
        return ptr_t(new Formatter("TODO: replace me some format string"));
      }

      virtual std::string format(const LogEvent &evt);
      std::string operator()(const LogEvent &evt) { return format(evt); }
      void setFormat(const std::string &fmt) { fmt_ = fmt; }
    private:
      std::string fmt_;
  }; /* -----  end of class Formatter  ----- */
}}

#endif   /* ----- #ifndef FHG_LOG_FORMATTER_HPP_INC  ----- */
