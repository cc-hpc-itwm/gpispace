/*
 * =====================================================================================
 *
 *       Filename:  WFSerialization.h
 *
 *    Description:  serialize/deserialize
 *
 *        Version:  1.0
 *        Created:  10/14/2009 03:34:47 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef GWDL_WFSERIALIZATION_HPP
#define GWDL_WFSERIALIZATION_HPP 1

#include <string>
#include <stdexcept>

#include <gwdl/IWorkflow.h>

namespace gwdl
{
  extern gwdl::IWorkflow *deserializeWorkflow(const std::string &) throw (std::runtime_error);
  extern std::string serializeWorkflow(const gwdl::IWorkflow &) throw (std::runtime_error);
}

#endif
