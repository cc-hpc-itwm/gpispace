/*
 * =====================================================================================
 *
 *       Filename:  warnings.hpp
 *
 *    Description:  some useful functions to remove warnings
 *
 *        Version:  1.0
 *        Created:  03/01/2010 02:24:56 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef _FHG_UTIL_WARNINGS_HPP
#define _FHG_UTIL_WARNINGS_HPP 1

namespace fhg { namespace util {
  template <typename T>
  inline void remove_unused_variable_warning(T) { }
}}

#endif
