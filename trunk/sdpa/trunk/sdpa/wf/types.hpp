#ifndef SDPA_WF_TYPES_HPP
#define SDPA_WF_TYPES_HPP 1

#include <list>
#include <sdpa/wf/WorkflowInterface.hpp>
#include <sdpa/wf/Activity.hpp>
using namespace std;

namespace sdpa {
	namespace wf {
		typedef std::string parameter_t;
		typedef std::list<parameter_t> parameter_list_t;
	}
}
#endif
