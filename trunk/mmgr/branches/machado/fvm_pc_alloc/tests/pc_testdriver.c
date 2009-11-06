/* The pc test driver loads different tests as the modules */
/* This tests module loading and runs different tests */

#include "modules.hpp"
#include "Module.hpp"
#include "ModuleLoader.hpp"



int main(int argc, char * argv[])
{
	//create a module loader
	ModuleLoader::ptr_t loader = ModuleLoader::create();
		
	  
// load module to work with fvm
	try {
		loader->load("pc-mod", "./libpc-mod.so");
	} catch (ModuleLoadFailed &mlf) ;

/* for i to ntests */
/* load test module */
/* run it */
/* unload it */

	
	return 0;
}
