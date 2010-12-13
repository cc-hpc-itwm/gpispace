#define BOOST_TEST_MODULE ParentPath
#include <boost/test/unit_test.hpp>

#include <boost/filesystem.hpp>

#include <vector>
#include <string>

#include <iostream>

BOOST_AUTO_TEST_CASE(strip_filename)
{
  typedef std::pair<std::string,std::string> path_type;
  typedef std::vector<path_type> paths_type;

  paths_type paths;

  paths.push_back (std::make_pair("",""));
  paths.push_back (std::make_pair(".","."));
  paths.push_back (std::make_pair("./","."));
  paths.push_back (std::make_pair("/level","/level"));
  paths.push_back (std::make_pair("/level/","/level"));
  paths.push_back (std::make_pair("/level/level","/level/level"));
  paths.push_back (std::make_pair("/level/level/","/level/level"));

  const std::string filename ("file.extension");

  for (paths_type::const_iterator p (paths.begin()); p != paths.end(); ++p)
    {
      const boost::filesystem::path path (p->first);
      const boost::filesystem::path cmp (p->second);
      const boost::filesystem::path file (path / filename);

      boost::filesystem::path path_without_filename (file);

      path_without_filename.remove_filename();

      BOOST_REQUIRE (path_without_filename == cmp);
    }
}
