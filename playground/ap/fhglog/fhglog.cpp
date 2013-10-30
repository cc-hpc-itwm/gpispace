#include <iostream>
#include <sstream>

struct log_stream : public std::ostringstream
{
  typedef log_stream this_type;

  log_stream ( const std::string & domain
             , const std::string & name
             , int level
             , const char * file
             , int line
             , const char * func
             )
    : do_it_(true)
  {
    os << domain << "(" << name << ")["<<level<<"] " << (file ? file : "nofile") << ":" << line << " " << (func ? func : "nofunc") << " - ";
  }

  log_stream (log_stream const & other)
    : os (other.os.str())
  {
    other.do_it_ = false;
  }

  log_stream & operator=(log_stream const & rhs)
  {
    if (this != &rhs)
    {
      os.str(rhs.os.str());
      rhs.do_it_ = false;
    }
    return *this;
  }

  ~log_stream ()
  {
    if (do_it_) std::cerr << os.str() << std::endl;
  }

  template <typename T>
  inline
  this_type & operator << ( const T & t )
  {
    os << t;
    return *this;
  }

  // make std::endl work
  inline
  this_type & operator << ( std::ostream & (*fn)(std::ostream &) )
  {
    fn (os);
    return *this;
  }

  std::ostringstream & stream ()
  {
    return *this;
  }
private:
  mutable bool do_it_;
  std::stringstream os;
};

struct nul_stream : public std::ostringstream
{
  typedef nul_stream this_type;

  static this_type get()
  {
    static this_type nil;
    return nil;
  }

  template <typename T>
  this_type & operator << ( const T & )
  {
    return *this;
  }

  this_type & operator << ( std::ostream & (*)(std::ostream &) )
  {
    return *this;
  }

  std::ostringstream & stream ()
  {
    return *this;
  }
};

template <typename T>
inline
nul_stream & operator << (nul_stream & ns, T const &)
{
  return ns;
}


struct logger
{
  explicit logger (std::string const & domain, std::string const & name)
    : domain_(domain)
    , name_(name)
  {
  }

  log_stream log(int level, const char * file = 0, int line = 0, const char * function = 0)
  {
    return log_stream(domain_, name_, level, file, line, function);
  }
private:
  std::string domain_;
  std::string name_;
};

#define CONC(a, b)
#define CONC_(a, b) a ## b
#define STR(x) STR_(x)
#define STR_(x) #x
#define FHGLOG_DECLARE_LOG(domain, name...)                     \
  struct fhg_log_logger_ ## domain                              \
  {                                                             \
    static logger & get()                                       \
    {                                                           \
      static logger log(#domain, #name);   \
      return log;                                               \
    }                                                           \
  }

FHGLOG_DECLARE_LOG( , );

#define LOG(level, domain...)                   \
  (level > 1 ? (fhg_log_logger_ ##domain::get().log(level, __FILE__, __LINE__, __FUNCTION__)).stream() : nul_stream::get().stream())

static std::string long_compute (int time)
{
  sleep (time);
  std::ostringstream os;
  os << time;
  return os.str();
}

int main()
{
  FHGLOG_DECLARE_LOG(main);
  FHGLOG_DECLARE_LOG(system_crit, crit);
  FHGLOG_DECLARE_LOG(system_mgmt, mgmt);

  logger test ("test", "blah");
  test.log(0, __FILE__, __LINE__, __FUNCTION__) << "hello world";

  fhg_log_logger_::get ().log (0, __FILE__, __LINE__, __FUNCTION__) << "test ";

  LOG(0, main) << "test main trace: " << long_compute(1);
  LOG(2, system_crit) << "critical";
  LOG(3) << "foo";
  (1 > 0 ? std::cout : std::cerr) << "foo";
}
