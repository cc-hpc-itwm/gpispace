#ifndef EVENT_H_
#define EVENT_H_
// std
#include <string>
#include <map>
// gwdl
#include "../../gworkflowdl_cpp/src/Data.h"

namespace gwes
{

/**
 * The Event encapsulates a notification message with is used by the Observer pattern.
 * Refer to GWES::attachWorkflowObserver().
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class Event
{
public:
	enum {
		
		/**
		 * Denotes that this is a general workflow event.
		 */
		EVENT_WORKFLOW = 1,
		
		/**
		 * Denotes that this is a general activity event.
		 */
		EVENT_ACTIVITY = 2,
		
		/**
		 * Denotes that this is an activity start event.
		 */
		EVENT_ACTIVITY_START = 3,
		
		/**
		 * Denotes that this is an activity end event.
		 */
		EVENT_ACTIVITY_END = 4
		
	};
	/**
	 * The type of the event. Can be:
	 * <ul>
	 * <li>EVENT_WORKFLOW = 1</li>
	 * <li>EVENT_ACTIVITY = 2</li>
	 * </ul>
	 */
	int _eventType;
	
	/**
	 * The identifier of the source of this event.
	 */
	std::string _sourceId;
	
	/**
	 * The message of this event.
	 */
	std::string _message;
	
	/**
	 * Vector that holds pointers to additional data, e.g., used as input parameters for 
	 * method calls.
	 * The first value contains the name or identifier of the data, the second value contains a 
	 * pointer to the Data object.
	 * Default is NULL.
	 */
	std::map<std::string,gwdl::Data*>* _dataP;
	
	/**
	 * Constructor for empty event
	 */
	Event() {}
	
	/**
	 * The constructor for this event.
	 * @param sourceId The identifier of the source of this event.
	 * @param eventType The type of the event (see _eventType).
	 * @param message The message of the event.
	 * @param dataP A pointer to a map which contains additional data. 
	 */
	Event(std::string sourceId, int eventType, std::string message, std::map<std::string,gwdl::Data*>* dataP = NULL) {
		_sourceId = sourceId; 
		_eventType=eventType; 
		_message = message; 
		_dataP = dataP;
	}
	
	/**
	 * The destructor for this event.
	 */
	virtual ~Event(){};
	
};

}

#endif /*EVENT_H_*/
