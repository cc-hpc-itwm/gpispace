#ifndef SDPA_WF_TYPES_HPP
#define SDPA_WF_TYPES_HPP 1

#include <list>
using namespace std;

namespace sdpa {
	namespace wf {
		typedef std::string workflow_t;
		typedef std::string activity_t;
		typedef std::string workflow_id_t;
		typedef std::string activity_id_t;
		typedef std::string parameter_t;
		typedef std::list<parameter_t> parameter_list_t;
	}
}
#endif
