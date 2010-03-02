#include "ModuleContainer.hpp"
#include <list>


// The ProcessContainer concept was seen, in the initial idea, as the
// entity that is running on a compute node and is responsible for
// loading of different code.

// It includes the functionality to communicate with the FVM (simulation
// or in real run) plus the so called NRE functionality (talking with the
// Aggregator). Still this NRE functionality might more than acting as a
// server and attending to requests from the Aggregator.  

// Therefore the question whether the ProcessContainer is: 

// 1 - the entity that acts just as a loader and loads code to
//  communicate with the FVM, the Aggregator and applications
//  (modules). This means the ProcessContainer is in fact _the_ NRE and is
//  started for loading of different code.

// 2 - the entity that (another) NRE creates to load modules and execute
// (eval) functions.


// The initial idea was more scenario 1. That is, a very thin layer, a
// loader of code for different things: communicate with the Aggregator
// in whatever fashion (different messaging,etc), communicate with the
// FVM (Infiniband,sockets,pipes,etc), elaborate memory managers,
// etc. This would allow totally different ProcessContainers each one
// acting differently, according to the different code it loads.

// Note that this was before or without looking to the name NRE. We can
// name the ProcessContainer as NRE and have different kinds of NRE's
// where the NRE is this layer that loads code.

// Still, scenario 2 is there for discussion as the big picture is still
// not settled.


class ProcessContainer {
public:
	ProcessContainer(std::string initialCode);
	~ProcessContainer();
	ModuleContainer load(const std::string pathname);
	int unload(ModuleContainer);

	//TODO:input and output need other types
	int eval(const std::string moduleName,const std::string functionName,int input,int output);

private:
	std::list<ModuleContainer> modulesList;
}
