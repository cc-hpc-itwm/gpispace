#include <fhglog/util.hpp>
#include <iterator>
#include <iostream>
#include <vector>

template <typename T>
struct default_handler
{
  void operator() (T const & actual, T const & expected, int pos)
  {
    std::cerr << "sequence comparison failed at position "
              << pos << ": "
              << "expected: " << expected << " but "
              << "got: " << actual
              << std::endl;
  }
};

template <typename I1, typename I2, typename Handler>
static int check_sequence ( I1 actual_begin
                          , I1 actual_end
                          , I2 expected_begin
                          , I2 expected_end
                          , Handler handler
                          )
{
  int errcount(0);

  int pos(0);
  while (actual_begin != actual_end && expected_begin != expected_end)
  {
    if (*actual_begin != *expected_begin)
    {
      handler(*actual_begin, *expected_begin, pos);
      ++errcount;
    }

    ++pos;
    ++actual_begin;
    ++expected_begin;
  }

  if (actual_begin != actual_end)
  {
    std::cerr << "sequence comparison failed: actual list not at end" << std::endl;
    ++errcount;
  }
  if (expected_begin != expected_end)
  {
    std::cerr << "sequence comparison failed: expected list not at end" << std::endl;
    ++errcount;
  }
  return errcount;
}

int main()
{
  int errcount(0);

  { // empty string -> empty path
    std::string path_string;
    std::vector<std::string> path;
    fhg::log::split (path_string, ".", std::back_inserter(path));
    std::vector<std::string> expected;
    check_sequence (path.begin(), path.end(), expected.begin(), expected.end(), default_handler<std::string>());
    std::copy(path.begin(), path.end(), std::ostream_iterator<std::string>(std::cout, "."));
  }

  // TODO: is this what we want?
  { // single . -> path[0] = ""
    std::string path_string(".");
    std::vector<std::string> path;
    fhg::log::split (path_string, ".", std::back_inserter(path));
    std::vector<std::string> expected;
    expected.push_back("");
    check_sequence (path.begin(), path.end(), expected.begin(), expected.end(), default_handler<std::string>());
    std::copy(path.begin(), path.end(), std::ostream_iterator<std::string>(std::cout, "."));
  }

  { // trailing .
    std::string path_string("foo.");
    std::vector<std::string> path;
    fhg::log::split (path_string, ".", std::back_inserter(path));
    std::vector<std::string> expected;
    expected.push_back("foo");
    check_sequence (path.begin(), path.end(), expected.begin(), expected.end(), default_handler<std::string>());
    std::copy(path.begin(), path.end(), std::ostream_iterator<std::string>(std::cout, "."));
  }

  { // components
    std::string path_string ("fhg.log.logger.1");
    std::vector<std::string> path;
    fhg::log::split (path_string, ".", std::back_inserter(path));
    std::vector<std::string> expected;
    expected.push_back("fhg");
    expected.push_back("log");
    expected.push_back("logger");
    expected.push_back("1");
    check_sequence (path.begin(), path.end(), expected.begin(), expected.end(), default_handler<std::string>());
    std::copy(path.begin(), path.end(), std::ostream_iterator<std::string>(std::cout, "."));
  }

  return errcount;
}
