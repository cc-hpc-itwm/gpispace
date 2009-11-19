/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef ACTIVITYTABLE_H_
#define ACTIVITYTABLE_H_
//gwes
#include <gwes/Activity.h>
#include <gwes/NoSuchActivityException.h>
//std
#include <string>
#include <map>

namespace gwes
{

/**
 * The ActivityTable holds all current available activities of a certain workflow.
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */ 
class ActivityTable : public std::map<std::string,Activity*> {

public:
	
	/**
	 * Constructor.
	 */
	ActivityTable();
	
	/**
	 * Destructor.
	 * Deletes also all activities inside the table.
	 */
	virtual ~ActivityTable();
	
	/**
	 * Put pointer of activity using its identifier as key to the table.
	 * @return The identifier of this activity.
	 */
	std::string put(Activity* activityP);
	
	/**
	 * Get pointer to specific activity.
	 * @param id The id of the activity.
	 */
	Activity* get(const std::string& id) throw (NoSuchActivityException);
	
	/** 
	 * Delete the activity and erase it from the table.
	 * @param activityID The identifier of the activity to erase.
	 */
	void remove(const std::string& activityID);

}; // end class ActivityTable

} // end namespace gwes

#endif /*ACTIVITYTABLE_H_*/
