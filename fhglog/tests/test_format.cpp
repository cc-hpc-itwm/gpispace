#include <fhglog/format.hpp>

int main()
{
  int errcount (0);

  using namespace fhg::log;

  {
    const std::string fmt("test %%");
    try
    {
      std::string msg (fhg::log::format(fmt, LogEvent()));
      if (msg != "test %")
      {
        throw std::runtime_error ("format error, expected \"test %\", got \""+msg+"\"");
      }
    }
    catch (std::exception const &ex)
    {
      ++errcount;
      std::cerr << "execption: " << ex.what() << std::endl;
    }
  }

  {
    try
    {
      fhg::log::check_format("test %-");
      ++errcount;
      std::cerr << "expected invalid format error!" << std::endl;
    }
    catch (std::exception const &ex)
    {
      // ok
    }
  }

  {
    try
    {
      std::string msg (fhg::log::format( fhg::log::default_format::SHORT()
                                       , LogEvent()
                                       )
                      );
    }
    catch (std::exception const &ex)
    {
      ++errcount;
      std::cerr << "execption: " << ex.what() << std::endl;
    }
  }

  return errcount;
}
