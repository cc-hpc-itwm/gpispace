#ifndef FHG_ASSERT_ASSERTION_FAILED_HPP
#define FHG_ASSERT_ASSERTION_FAILED_HPP

#include <exception>
#include <sstream>
#include <string>

namespace fhg
{
  namespace assert_helper
  {
    inline std::string message ( std::string const & cond
                               , std::string const & mesg
                               , std::string const & file
                               , int line
                               )
    {
      std::ostringstream sstr;
      sstr << "assertion '" << cond << "'"
           << " in " << file << ":" << line
           << " failed";
      if (not mesg.empty())
      {
        sstr << ": " << mesg;
      }
      return sstr.str();
    }
  };

  class assertion_failed : public std::logic_error
  {
  public:
    assertion_failed ( std::string const & cond
                     , std::string const & message
                     , std::string const & file
                     , int line
                     )
      : std::logic_error (assert_helper::message(cond, message, file, line))
      , m_cond (cond)
      , m_message (message)
      , m_file (file)
      , m_line (line)
    {}

    virtual ~assertion_failed() throw() = default;

    std::string const & condition() const { return m_cond; }
    std::string const & message() const { return m_message; }
    std::string const & file() const { return m_file; }
    int line() const { return m_line; }
  private:
    const std::string m_cond;
    const std::string m_message;
    const std::string m_file;
    const int         m_line;
  };
}

#endif
