#include "ModuleContainer.hpp"


// TODO: LOTS of error/return value checking

ModuleContainer::ModuleContainer(std::string sFilename) :
	m_sFilename(sFilename)
{
	m_pHandle = _dynl_open();
	if(m_pHandle==NULL) throw;
	refcount++;

	const char *sSymbol = "mod_init";
	m_pFuncInit = (fktn2_t*) _dynl_sym(sSymbol);
	if(m_pFuncInit == NULL) throw;

	(*m_pFuncInit)(this);
}

ModuleContainer::~ModuleContainer()
{
	_dynl_close();
}

void *ModuleContainer::_dynl_open()  {
	return(dlopen(m_sFilename.c_str(), RTLD_NOW|RTLD_GLOBAL));
}

void *ModuleContainer::_dynl_sym(const char *sSymbol) {
	return(dlsym(m_pHandle, sSymbol));
}

int ModuleContainer::_dynl_close()
{
	return(dlclose (m_pHandle));
}
