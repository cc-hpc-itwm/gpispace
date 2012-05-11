#ifndef FHG_ASSERT_ASSERTION_FAILED_HPP
#define FHG_ASSERT_ASSERTION_FAILED_HPP

#include <exception>

namespace fhg
{
  class assertion_failed : public std::exception
  {
  public:
    static std::string make_what_text ( std::string const & cond
                                      , std::string const & message
                                      , std::string const & file
                                      , int line
                                      )
    {
      std::ostringstream sstr;
      sstr << "assertion '" << cond << "'"
           << " in " << file << ":" << line
           << " failed: " << message;
      return sstr.str();
    }

    assertion_failed ( std::string const & cond
                     , std::string const & message
                     , std::string const & file
                     , int line
                     )
      : m_cond (cond)
      , m_message (message)
      , m_file (file)
      , m_line (line)
      , m_what_text (make_what_text(cond, message, file, line))
    {}

    virtual ~assertion_failed() throw() {}

    const char * what () const throw() { return m_what_text.c_str(); }

    std::string const & condition() const { return m_cond; }
    std::string const & message() const { return m_message; }
    std::string const & file() const { return m_file; }
    int line() const { return m_line; }
  private:
    const std::string m_cond;
    const std::string m_message;
    const std::string m_file;
    const int         m_line;
    const std::string m_what_text;
  };
}

#endif
