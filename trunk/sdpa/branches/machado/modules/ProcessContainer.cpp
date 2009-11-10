#include "ProcessContainer.hpp"


// TODO: in all functions we iterate for the module over the list->NOT good
//maybe a faster structure

ProcessContainer::ProcessContainer(std::string initialCode)
{

	//create modulesList
	modulesList = new list<ModuleContainer>;


	//TODO: do we have initial code, then use

}

ProcessContainer::~ProcessContainer()
{
	delete modulesList;
}

ModuleContainer load(const std::string pathname) 
{

	list<ModuleContainer>::iterator i = modulesList.begin();
	for(int j = 0; j < modulesList.size() ; j++){
		//if found
		if((ModuleContainer)(*i).getFileName.compare(pathname) == 0){
			//increase reference count;
			(ModuleContainer)(*i).incRefCount();
			//return object
			return (ModuleContainer)(*i);
		}
		i++;
	}
	//not found
	ModuleContainer mod = new ModuleContainer(pathname);

	//TODO: check if mod is sane (mod != NULL enough? )

	modulesList.push_back(mod);
	return mod;
	
}

bool unload(ModuleContainer mod) 
{

	list<ModuleContainer>::iterator i = modulesList.begin();
	for(int j = 0; j < modulesList.size() ; j++)
		if(*i == mod){
			//decrease reference count;
			*i.decRefCount();
			//assert (refcount > 0);

			if (*i.getRefCount == 0) {
				//TODO: remove from list;
				
				delete mod;

			}
			return true;
		}

#ifndef NDEBUG
	//clear out the referenced memory with zeros to hopefully provoke
	//a segfault when accessing it
#endif

	return false;


}

//TODO: a bool return value does not tell us what happened:
//module not loaded or function not found?

//TODO: again iterating over list and then even
// doing string compare -> ugly!!!


//TODO: everytime a function is used 
//we could put it in a function cache for faster look-up
//after a few uses of the module, the look-up would be faster

bool eval(moduleName,functionName,input,output) {

	list<ModuleContainer>::iterator i = modulesList.begin();
	for(int j = 0; j < modulesList.size() ; j++){
		//if found
		if((ModuleContainer)(*i).getFileName().compare(moduleName) == 0){
			
			void * f = (ModuleContainer)(*i).getFunction(functionName); x
			if(f != NULL){
				//execute function
				//TODO: f must always have the same signature
				f(input,output);
				return true
			}
			else
				//function not found;
				return false;
			i++;
		}
	}
	//module not loaded;
	//load it in case not loaded?
	return false;
}
