#ifndef ACTIVITYEXCEPTION_H_
#define ACTIVITYEXCEPTION_H_
// std
#include <string>

namespace gwes
{

/**
 * An ActivityException is thrown if an error during the activity execution occurs.
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class ActivityException {
	
	public:
		std::string message;
		ActivityException(std::string _message) 
		{
			message = _message;					
		}
}; // end class ActivityException

} // end namespace gwes

#endif /*ACTIVITYEXCEPTION_H_*/
