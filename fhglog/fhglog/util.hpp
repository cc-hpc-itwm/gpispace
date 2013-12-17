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

#include <boost/filesystem.hpp>

namespace fhg { namespace log {
  inline std::string get_filename_from_path(const std::string &a_path)
  {
    return boost::filesystem::path (a_path).filename().string();
  }

  inline std::string get_module_name_from_path(const std::string &a_path)
  {
    return boost::filesystem::path (a_path).stem().string();
  }
}}

#endif
