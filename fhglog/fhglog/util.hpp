/*
 * =====================================================================================
 *
 *       Filename:  util.hpp
 *
 *    Description:  some utility functions
 *
 *        Version:  1.0
 *        Created:  09/29/2009 02:48:49 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef FHGLOG_UTIL_HPP
#define FHGLOG_UTIL_HPP 1

#include <string>
#include <algorithm> // std::transform
#include <cctype> // std::tolower

#include <list>
#include <string>
#include <sstream> // stringstream
#include <utility> // std::pair
#include <unistd.h> // char **environ

namespace fhg { namespace log {
  typedef std::pair<std::string, std::string> env_value_t;
  typedef std::list<env_value_t> environment_t;

  inline std::string get_filename_from_path(const std::string &a_path)
  {
    // TODO: the following should be coded with boost::filesystem due to platform
    // independence
    static const std::string path_sep("/");
    std::string::size_type last_path_segment_idx = a_path.find_last_of(path_sep.c_str());
    if (last_path_segment_idx == std::string::npos)
    {
      // return the whole path since we could not find a separator
      return a_path;
    }
    else
    {
      // slit the path at that position and return the remaining part
      return a_path.substr(last_path_segment_idx+1);
    }
  }

  inline std::string get_module_name_from_path(const std::string &a_path)
  {
    std::string filename(get_filename_from_path(a_path));
    std::transform(filename.begin(), filename.end(), filename.begin(), tolower);

    std::string::size_type ext_start_idx = filename.find_last_of(".");
    if (ext_start_idx == std::string::npos)
    {
      // return the whole path since we could not find a separator
      return filename;
    }
    else
    {
      // split the filename and return everything up to the last "."
      return filename.substr(0, ext_start_idx);
    }
  }


  template <typename Iterator>
  inline std::string join ( Iterator begin
                          , Iterator end
                          , const std::string & separator
                          , const std::string & lead_in = ""
                          , const std::string & lead_out = ""
                          )
  {
    std::ostringstream os;

    os << lead_in;

    Iterator it = begin;
    while (it != end)
    {
      if (it != begin)
      {
        os << separator;
      }
      os << *it;
      ++it;
    }

    os << lead_out;

    return os.str();
  }

    environment_t get_environment_variables();
    unsigned int gettid();
}}

#endif
