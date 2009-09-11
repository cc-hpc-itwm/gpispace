#ifndef SDPA_WF_HPP
#define SDPA_WF_HPP 1

#include <sdpa/wf/types.hpp>

// To be replaced with a real definition/implementation of a workflow
// Below, the functionality required by sdpa
namespace sdpa { namespace wf {

typedef std::string workflow_id_t;

class Workflow
{
public:
	typedef Workflow workflow_t;
	Workflow(std::string desc) { description = desc; }
	workflow_id_t getId() const { return id; }
	workflow_id_t setId(const workflow_id_t& workflow_id) { id = workflow_id; }

	std::string serialize() const { return "dummy workflow"; }

private:
	workflow_id_t id;
	std::string description;
};

}}

#endif
