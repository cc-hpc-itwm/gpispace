#ifndef NOSUCHWORKFLOWEXCEPTION_H_
#define NOSUCHWORKFLOWEXCEPTION_H_
// std
#include <string>

namespace gwes
{

/**
 * A NoSuchWorkflowException is thrown if the workflow is not available.
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class NoSuchWorkflowException {
	
	public:
		std::string message;
		NoSuchWorkflowException(std::string _message) 
		{
			message = _message;					
		}
}; // end class NoSuchWorkflowException

} // end namespace gwes

#endif /*NOSUCHWORKFLOWEXCEPTION_H_*/
