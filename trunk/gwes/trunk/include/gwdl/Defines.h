#ifndef DEFINES_H_
#define DEFINES_H_

#define WORKFLOW_DEFAULT_DESCRIPTION ""
#define WORKFLOW_DEFAULT_ID "No_ID"

#define SCHEMA_wfSpace "http://www.gridworkflow.org/gworkflowdl"
#define SCHEMA_location "http://www.gridworkflow.org/gworkflowdl http://www.gridworkflow.org/kwfgrid/src/xsd/gworkflowdl_2_0.xsd"
#define SCHEMA_xsi "http://www.w3.org/2001/XMLSchema-instance"
#define SCHEMA_xsd "http://www.w3.org/2001/XMLSchema"
#define SCHEMA_wf_oc "http://www.gridworkflow.org/gworkflowdl/operationclass"


/**
 * This class contains global definitions for the GWorkflowDL-API. 
 * 
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>
 */  
class Defines
{
public:
	Defines() {}
	virtual ~Defines();
};

#endif /*DEFINES_H_*/
