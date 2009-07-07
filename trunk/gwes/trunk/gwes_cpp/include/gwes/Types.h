#ifndef TYPES_H_
#define TYPES_H_
// gwdl
#include <gwdl/Workflow.h>
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
typedef gwdl::Workflow workflow_t;

/**
 * Type of the workflow identifier.
 */
typedef std::string workflow_id_t;

/**
 * Type for referencing to a SPDA activity.
 * ToDo: replace by specific SPDA activity class. Meanwhile using gwes::Activity.
 */
typedef Activity activity_t;

/**
 * Type fo the activity identifier.
 */
typedef std::string activity_id_t;

/**
 * Type of input/output parameters.
 */
typedef std::string parameter_t;

/**
 * Type of tuple of parameters.
 */
typedef std::list<parameter_t> parameter_list_t;

} // end namespace gwes

#endif /*TYPES_H_*/
