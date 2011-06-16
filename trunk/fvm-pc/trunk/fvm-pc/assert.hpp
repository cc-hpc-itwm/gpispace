/*
 * =====================================================================================
 *
 *       Filename:  assert.hpp
 *
 *    Description:  defines some functions that check memory handles
 *
 *        Version:  1.0
 *        Created:  11/16/2009 12:53:55 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef SDPA_MODULES_ASSERT_HPP
#define SDPA_MODULES_ASSERT_HPP 1

#include <fhglog/fhglog.hpp>
#include <fvm-pc/pc.hpp>
#include <stdexcept>
#include <stdio.h>

namespace fvm
{
  namespace util
  {
    inline void assert_alloc(const fvmAllocHandle_t hdl, const std::string &hdl_name, const std::string &tag) throw (std::exception)
    {
      if (! hdl)
      {
        const std::string error("allocation ("+tag+") for handle " + hdl_name + " failed!");
        MLOG (ERROR, error);
        throw std::runtime_error(error);
      }
    }

    inline void assert_valid_handle(const fvmAllocHandle_t hdl, const std::string &hdl_name) throw (std::exception)
    {
      if (! hdl)
      {
        const std::string error("invalid memory handle: " + hdl_name);
        MLOG (ERROR, error);
        throw std::runtime_error(error);
      }
    }

    inline void assert_success(const int return_value, const std::string &tag) throw (std::exception)
    {
      if (return_value != 0)
      {
        char return_value_string[64];
        snprintf( return_value_string, sizeof(return_value_string), "%d", return_value);

        const std::string error("operation failed ("+tag+"): " + return_value_string);
        MLOG (ERROR, error);
        throw std::runtime_error(error);
      }
    }

#define ASSERT_ALLOC(hdl, tag) ::fvm::util::assert_alloc(hdl, #hdl, tag)
#define ASSERT_GALLOC(hdl) ASSERT_ALLOC(hdl, "global")
#define ASSERT_LALLOC(hdl) ASSERT_ALLOC(hdl, "local")

#define ASSERT_HANDLE(hdl) ::fvm::util::assert_valid_handle(hdl, #hdl)

#define ASSERT_SUCCESS(retval, tag) ::fvm::util::assert_success(retval, tag)

  }
}

#endif
