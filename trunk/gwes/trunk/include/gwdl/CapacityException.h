#ifndef CAPACITYEXCEPTION_H_
#define CAPACITYEXCEPTION_H_
#include <string>

using namespace std;

namespace gwdl
{

/**
 * A CapacityException is thrown when the number of tokens on a place exceeds its capacity.
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class CapacityException {
	
	public:
		string message;
		CapacityException(string _message) 
		{
			message = _message;					
		}
}; // end class CapacityException

} // end namespace gwdl

#endif /*CAPACITYEXCEPTION_H_*/
