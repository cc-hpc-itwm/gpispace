#ifndef TESTWORKFLOW_H_
#define TESTWORKFLOW_H_
// std
#include <string>
// gwdl
#include "../../gworkflowdl_cpp/src/Workflow.h"
// gwes
#include "../../gwes_cpp/src/GWES.h"

gwdl::Workflow& testWorkflow(std::string workflowfn, gwes::GWES &gwes);

#endif /*TESTWORKFLOW_H_*/
