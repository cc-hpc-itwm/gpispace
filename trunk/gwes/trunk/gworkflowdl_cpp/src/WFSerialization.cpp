/*
 * =====================================================================================
 *
 *       Filename:  WFServialization.cpp
 *
 *    Description:  serialize/deserialize
 *
 *        Version:  1.0
 *        Created:  10/14/2009 03:36:34 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <string>
#include <sstream>

#include <gwdl/WFSerialization.h>
#include <gwdl/XMLUtils.h>
#include <gwdl/Workflow.h>

namespace gwdl
{
gwdl::IWorkflow *deserialize(const std::string &bytes)
{
  XERCES_CPP_NAMESPACE::DOMElement* element = (gwdl::XMLUtils::Instance()->deserialize(bytes))->getDocumentElement();
  return new gwdl::Workflow(element);
}

std::string serialize(const gwdl::IWorkflow &workflow)
{
  gwdl::Workflow &real_workflow = static_cast<gwdl::Workflow&>(const_cast<IWorkflow&>(workflow));
  std::ostringstream ostr;
  ostr << real_workflow;
  return ostr.str();
}
}
