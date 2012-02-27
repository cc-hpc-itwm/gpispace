#include <fhglog/logger_path.hpp>
#include <iostream>

int main()
{
  int errcount (0);

  using namespace fhg::log;

  logger_path p("fhg.logger.test");

  {
    std::string expected ("fhg.logger.test");
    if (p.str() != expected)
    {
      std::cerr << "*** failed: expected: " << expected << " got: " << p.str() << std::endl;
      errcount += 1;
    }
  }

  {
    std::string expected ("fhg.logger.test.id.1");
    logger_path p_id ( p / "id" / 1 );
    if (p_id.str() != expected)
    {
      std::cerr << "*** failed: expected: " << expected << " got: " << p_id.str() << std::endl;
      errcount += 1;
    }
  }

  {
    std::stringstream sstr;
    sstr << p;
    logger_path p_in;
    sstr >> p_in;

    if (p_in != p)
    {
      std::cerr << "*** failed: expected: " << p << " got: " << p_in << std::endl;
      errcount += 1;
    }
  }

  return errcount;
}
