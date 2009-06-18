#ifndef NOSUCHACTIVITYEXCEPTION_H_
#define NOSUCHACTIVITYEXCEPTION_H_
#include <string>

namespace gwes
{

/**
 * A NoSuchActivityException is thrown if the activity is not available.
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class NoSuchActivityException {
	
	public:
		std::string message;
		NoSuchActivityException(std::string _message) 
		{
			message = _message;					
		}
}; // end class NoSuchActivityException

} // end namespace gwes

#endif /*NOSUCHACTIVITYEXCEPTION_H_*/

