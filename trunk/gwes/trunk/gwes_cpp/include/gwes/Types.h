#ifndef TYPES_H_
#define TYPES_H_
// gwes
#include <gwes/TokenParameter.h>
#include <gwes/IActivity.h>
// gwdl
#include <gwdl/IWorkflow.h>
// std
#include <string>
#include <list>

namespace gwes
{

class Activity;

/**
 * Type for referencing to a GWorkflowDL workflow.
 * @todo: It could be better to use a reference to the corresponding WorkflowHandler instead!
 */
typedef gwdl::IWorkflow workflow_t;

/**
 * Type of the workflow identifier.
 */
typedef gwdl::IWorkflow::workflow_id_t workflow_id_t;

/**
 * Type for referencing to a SPDA activity.
 * ToDo: replace by specific SPDA activity class. Meanwhile using gwes::Activity.
 */
typedef IActivity activity_t;

/**
 * Type fo the activity identifier.
 */
typedef IActivity::activity_id_t activity_id_t;

/**
 * Type of read/input/write/output parameters.
 */
typedef TokenParameter parameter_t;

/**
 * Type of tuple of parameters.
 * The list is sorted by scope:
 * First read tokens
 * Second input tokens
 * Third write tokens
 * Forth output tokens.
 */
typedef std::list<parameter_t> parameter_list_t;

/**
 * Type of map with edgeExpressions as keys and token pointers as values.
 */
////typedef std::map<std::string,gwdl::Token*> token_map_t;

} // end namespace gwes

#endif /*TYPES_H_*/
