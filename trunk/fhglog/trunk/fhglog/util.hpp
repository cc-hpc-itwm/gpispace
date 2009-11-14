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

  inline environment_t get_environment_variables()
  {
      environment_t env;
      char ** env_p = environ;
      while (env_p != NULL && (*env_p != NULL))
      {
        const std::string env_entry(*env_p);
        // split at =
        std::string::size_type split_pos = env_entry.find("=");

        const std::string key = env_entry.substr(0, split_pos);
        if (split_pos != std::string::npos)
        {
          env.push_back(std::make_pair(key, env_entry.substr(split_pos+1)));
        }
        else
        {
          env.push_back(std::make_pair(key, ""));
        }
        ++env_p;
      }
      return env;
  }
}}

#endif
