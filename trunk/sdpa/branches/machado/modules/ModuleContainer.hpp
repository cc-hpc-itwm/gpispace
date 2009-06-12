// The ModuleContainer holds the information for the .so file and is responsible
// for the actions on the module.

class ModuleContainer;

typedef int (*fktn2_t)(ModuleContainer*);
typedef std::map<const std::string, FunctionContainer, ltstr>  MAP_FKTN_T;

class ModuleContainer {
public:
        typedef sdpa::shared_ptr<ModuleContainer> Ptr;

	ModuleContainer(std::string sFilename);
	~ModuleContainer();
	void *getFunction(
	void incRefCount();
	void decRefCount();
	int getRefCount();
	std::string getFileName();
	void * getFunction(std::string functionName);

private:
	void *_dynl_open();
	void *_dynl_sym(const char *sSymbol);
	int _dynl_close();


private:
	std::string filename; 
	std::string fullpath;
	void *m_pHandle;             /**< modul handler (to shared object) */
	int refcount; //reference counting
	//	fktn2_t *m_pFuncInit;        /**< pointer to initialisation function */
	//	MAP_FKTN_T m_Function_Map;   /**< map of valid functions, filled by pFuncInit() */
};
