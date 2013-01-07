/*
 * =====================================================================================
 *
 *       Filename:  revision.hpp
 *
 *    Description:  just contains the revision information of the project
 *
 *        Version:  1.0
 *        Created:  02/20/2011 01:06:13 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef FHG_UTIL_REVISION_HPP
#define FHG_UTIL_REVISION_HPP 1

#include <string>

namespace fhg
{
  extern const char* project_contact();
  extern const char* project_summary();
  extern const char* project_copyright();
  extern const char* project_version();
  extern const char* project_revision();
  extern const char* project_build_time();
  extern const char* project_build_compiler();
  extern const char* project_build_info();
  extern const char* project_build_count();

  extern std::string project_info (const std::string&);
}

#endif
